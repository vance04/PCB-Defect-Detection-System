#ifndef INFERENCE_H
#define INFERENCE_H

#include <vector>
#include <string>
#include <random>
#include <opencv2/opencv.hpp>
#include <onnxruntime_cxx_api.h>

struct Detection {
    int class_id{0};
    std::string className{};
    float confidence{0.0};
    cv::Scalar color{};
    cv::Rect box{};
};

class Inference {
public:
    Inference(const std::string &onnxModelPath,
              const cv::Size &modelInputShape = {320, 320},
              bool runWithCuda = false);
    ~Inference();

    std::vector<Detection> runInference(const cv::Mat &input);

private:
    void loadOnnxNetwork();
    cv::Mat formatToSquare(const cv::Mat &source);
    cv::Scalar generateRandomColor(int classId);

    std::string modelPath;
    bool cudaEnabled;
    std::vector<std::string> classes;
    cv::Size modelShape;

    float modelConfidenceThreshold {0.25};
    float modelScoreThreshold {0.45};
    float modelNMSThreshold {0.50};
    bool letterBoxForSquare = true;

    // ONNX Runtime members
    Ort::Env env;
    Ort::SessionOptions sessionOptions;
    Ort::Session session{nullptr};
    Ort::AllocatorWithDefaultOptions allocator;
    std::vector<const char*> inputNames;
    std::vector<const char*> outputNames;
};

#endif // INFERENCE_H
