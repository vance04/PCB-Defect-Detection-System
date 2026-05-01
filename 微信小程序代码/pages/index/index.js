Page({
  data: {
    Temp:0,
    Hum:0,
    Light:0,
    DefectTotalCount:0,
    OpenCircuit:0,
    Short:0,
    Spur:0,
     // ========== 记录列表和控制变量 ==========
     records: [],          // 存储历史记录，每条记录包含时间及各项数据
     showRecords: false,   // 控制记录列表显示/隐藏
     maxRecords: 40 ,       // 最多保存20条记录，超出自动删除最早记录
     lastDefectTotalCount: -1   // 初始-1表示尚未记录过
  },
    
  config: {
      authorization: "", // 鉴权信息
      product_id: "", // 产品ID
      device_name: "", // 设备名称      
      getinfo_url: '', //设备属性最新数据查询地址
      setinfo_url:''//设置设备属性
    },
  Onenet_GetInfo()
  {
    wx.request({
      url: this.config.getinfo_url,
      header:{
        'authorization':this.config.authorization
      },
      method:"GET",
      success: (e) =>
      {
        console.log(e);
        // 获取最新数据
        const newData = {
          Temp: e.data.data[2].value,
          Light: e.data.data[3].value,
          Hum: e.data.data[0].value,
          DefectTotalCount: e.data.data[1].value,
          OpenCircuit: e.data.data[4].value,
          Short: e.data.data[5].value,
          Spur: e.data.data[6].value
        };
         // ========== 判断是否有新增瑕疵（缺陷总数增加） ==========
         const oldTotal = this.data.lastDefectTotalCount;
         const newTotal = newData.DefectTotalCount;
         // 首次加载时 oldTotal === -1 不记录
         if (oldTotal !== -1 && newTotal > oldTotal) {
           // 有新瑕疵：添加一条记录，记录当前累计值（自然包含了本次的增量）
           const now = new Date();
           const timeStr = `${now.getFullYear()}-${now.getMonth()+1}-${now.getDate()} ${now.getHours()}:${now.getMinutes()}:${now.getSeconds()}`;
           const record = {
             time: timeStr,
             DefectTotalCount: newData.DefectTotalCount,
             OpenCircuit: newData.OpenCircuit,
             Short: newData.Short,
             Spur: newData.Spur
           };
           let newRecords = [record, ...this.data.records];
           if (newRecords.length > this.data.maxRecords) {
             newRecords = newRecords.slice(0, this.data.maxRecords);
           }
           this.setData({ records: newRecords });
         }
        this.setData({
          ...newData,
          lastDefectTotalCount: newTotal   // 更新上一次的总数

        })
      }
    })
  },
  Onenet_SetMotorInfo(event)
  {
    const is_checked = event.detail.value; // 获取开关状态
    wx.showToast({
      title: '操作成功', // 提示的文字内容
      icon: 'success', // 图标类型，使用成功图标
      duration: 1000 // 提示框自动隐藏的时间，单位是毫秒
    }), 
    wx.request({
      url: this.config.setinfo_url,
      header:{
        'authorization':this.config.authorization
      },
      method:"POST",
      data: {
        "product_id": this.config.product_id,
        "device_name": this.config.device_name,
        "params": {
          "motor": is_checked
        }     
      },            
    })
  },    

// ========== 新增：切换记录列表显示/隐藏 ==========
toggleRecords() {
  this.setData({
    showRecords: !this.data.showRecords
  });
},

  onLoad(){
    //每五秒调用一次
     // ========== 使用 bind(this) 确保函数内 this 指向正确 ==========
     setInterval(this.Onenet_GetInfo.bind(this), 5000);
  }
})
