#include "inference.h"
#include <numeric>
#include <algorithm>

Inference::Inference(const std::string &onnxModelPath,
                     const cv::Size &modelInputShape,
                     bool runWithCuda)
    : modelPath(onnxModelPath),
    cudaEnabled(runWithCuda),
    modelShape(modelInputShape),
    env(ORT_LOGGING_LEVEL_WARNING, "ONNXRuntime")
{
    // classes = {"missing_hole", "mouse_bite", "open_circuit", "short", "spurious_copper", "spur" , "damaged" };
    classes = {"missing_hole", "mouse_bite", "open_circuit", "short", "spurious_copper", "spur"  };    
    loadOnnxNetwork();
}

Inference::~Inference() {
    for (auto name : inputNames) delete[] name;
    for (auto name : outputNames) delete[] name;
}

cv::Scalar Inference::generateRandomColor(int classId) {
    std::mt19937 rng(classId);
    std::uniform_int_distribution<int> dist(0, 255);
    return cv::Scalar(dist(rng), dist(rng), dist(rng));
}

std::vector<Detection> Inference::runInference(const cv::Mat &input) {
    std::vector<Detection> detections;

    // Step 1: 图像预处理
    cv::Mat modelInput = input;
    if (letterBoxForSquare && modelShape.width == modelShape.height) {
        modelInput = formatToSquare(modelInput);
    }

    // Step 2: 创建blob
    cv::Mat blob;
    cv::dnn::blobFromImage(
        modelInput, blob, 1.0/255.0, modelShape, cv::Scalar(), true, false, CV_32F);

    // Step 3: 创建ONNX输入张量
    std::vector<int64_t> inputShape = {1, 3, modelShape.height, modelShape.width};
    Ort::MemoryInfo memoryInfo = Ort::MemoryInfo::CreateCpu(OrtArenaAllocator, OrtMemTypeDefault);
    Ort::Value inputTensor = Ort::Value::CreateTensor<float>(
        memoryInfo, blob.ptr<float>(), blob.total(), inputShape.data(), inputShape.size());

    // Step 4: 运行推理
    std::vector<Ort::Value> outputTensors = session.Run(
        Ort::RunOptions{nullptr},
        inputNames.data(),
        &inputTensor,
        1,
        outputNames.data(),
        outputNames.size()
        );

    // Step 5: 解析输出
    float* outputData = outputTensors[0].GetTensorMutableData<float>();
    auto outputShape = outputTensors[0].GetTensorTypeAndShapeInfo().GetShape();

    // YOLOv8输出格式处理
    int rows = outputShape[1];  // 通常是84x8400
    int dimensions = outputShape[2];

    // 转置输出矩阵为[8400,84]
    cv::Mat outputMat = cv::Mat(rows, dimensions, CV_32F, outputData);
    outputMat = outputMat.t();

    float x_factor = modelInput.cols / static_cast<float>(modelShape.width);
    float y_factor = modelInput.rows / static_cast<float>(modelShape.height);

    std::vector<int> class_ids;
    std::vector<float> confidences;
    std::vector<cv::Rect> boxes;

    for (int i = 0; i < outputMat.rows; ++i) {
        float* row = outputMat.ptr<float>(i);

        // 提取类别概率 (跳过前4个bbox值)
        cv::Mat scoresMat(1, classes.size(), CV_32F, row + 4);
        cv::Point class_id;
        double maxClassScore;
        cv::minMaxLoc(scoresMat, 0, &maxClassScore, 0, &class_id);

        if (maxClassScore > modelScoreThreshold) {
            confidences.push_back(static_cast<float>(maxClassScore));
            class_ids.push_back(class_id.x);

            float x = row[0];
            float y = row[1];
            float w = row[2];
            float h = row[3];

            int left = static_cast<int>((x - 0.5 * w) * x_factor);
            int top = static_cast<int>((y - 0.5 * h) * y_factor);
            int width = static_cast<int>(w * x_factor);
            int height = static_cast<int>(h * y_factor);

            boxes.emplace_back(left, top, width, height);
        }
    }

    // 非极大值抑制 (NMS)
    std::vector<int> nms_result;
    cv::dnn::NMSBoxes(boxes, confidences, modelScoreThreshold, modelNMSThreshold, nms_result);

    // 创建最终的检测结果
    for (int idx : nms_result) {
        Detection detection;
        detection.class_id = class_ids[idx];
        detection.confidence = confidences[idx];
        detection.className = classes[detection.class_id];
        detection.box = boxes[idx];
        detection.color = generateRandomColor(detection.class_id);

        detections.push_back(detection);
    }

    return detections;
}

//加载权重网络模块
void Inference::loadOnnxNetwork() {
    if (cudaEnabled) {
        OrtCUDAProviderOptions cuda_options;
        sessionOptions.AppendExecutionProvider_CUDA(cuda_options);
    }

    sessionOptions.SetGraphOptimizationLevel(GraphOptimizationLevel::ORT_ENABLE_ALL);
    session = Ort::Session(env, modelPath.c_str(), sessionOptions);

    // 获取输入输出名称
    Ort::AllocatedStringPtr inputName = session.GetInputNameAllocated(0, allocator);
    Ort::AllocatedStringPtr outputName = session.GetOutputNameAllocated(0, allocator);

    inputNames.push_back(strdup(inputName.get()));
    outputNames.push_back(strdup(outputName.get()));
}

cv::Mat Inference::formatToSquare(const cv::Mat &source) {
    int col = source.cols;
    int row = source.rows;
    int _max = std::max(col, row);
    cv::Mat result = cv::Mat::zeros(_max, _max, CV_8UC3);
    source.copyTo(result(cv::Rect(0, 0, col, row)));
    return result;
}
