document.addEventListener("DOMContentLoaded", function () {

  if (typeof Chart === "undefined") {
    console.error("Chart.js chưa load!");
    return;
  }

  const labels = [];
  const tempData = [];
  const humData = [];

  let lastTemp = null;
  let lastHum = null;

  const tempCanvas = document.getElementById("tempChart");
  const humCanvas = document.getElementById("humChart");

  if (!tempCanvas || !humCanvas) {
    console.error("Không tìm thấy canvas!");
    return;
  }

  // Khởi tạo biểu đồ nhiệt độ
  const tempChart = new Chart(tempCanvas, {
    type: "line",
    data: {
      labels: labels,
      datasets: [{
        label: "Nhiệt độ (°C)",
        data: tempData,
        borderColor: "red",
        backgroundColor: "rgba(255,0,0,0.2)",
        fill: true
      }]
    },
    options: {
      responsive: true,
      maintainAspectRatio: false,
      scales: {
        y: {
          suggestedMin: Math.min(...tempData) - 1,
          suggestedMax: Math.max(...tempData) + 1,
          ticks: { stepSize: 1 }
        },
        x: {
          ticks: {
            callback: function(value, index) {
              return index % 2 === 0 ? this.getLabelForValue(value) : '';
            }
          }
        }
      }
    }
  });

  // Khởi tạo biểu đồ độ ẩm
  const humChart = new Chart(humCanvas, {
    type: "line",
    data: {
      labels: labels,
      datasets: [{
        label: "Độ ẩm (%)",
        data: humData,
        borderColor: "blue",
        backgroundColor: "rgba(0,0,255,0.2)",
        fill: true
      }]
    },
    options: {
      responsive: true,
      maintainAspectRatio: false,
      scales: {
        y: {
          suggestedMin: Math.min(...humData) - 1,
          suggestedMax: Math.max(...humData) + 1,
          ticks: { stepSize: 0.5 }
        },
        x: {
          ticks: {
            callback: function(value, index) {
              return index % 2 === 0 ? this.getLabelForValue(value) : '';
            }
          }
        }
      }
    }
    
  });

  // Hàm cập nhật dữ liệu cảm biến
  function updateSensors() {
    fetch('/sensors')
      .then(response => response.json())
      .then(data => {
        // Hiển thị số liệu
        document.getElementById('temp').innerText = data.temp + " °C";
        document.getElementById('hum').innerText = data.hum + " %";

        // Thêm nhãn thời gian (ví dụ giờ hiện tại)
        const now = new Date().toLocaleTimeString();
        labels.push(now);

        // Thêm dữ liệu mới
        tempData.push(data.temp);
        humData.push(data.hum);

        // Giới hạn số điểm hiển thị (ví dụ 10 điểm gần nhất)
        if (labels.length > 10) {
          labels.shift();
          tempData.shift();
          humData.shift();
        }

        // Cập nhật biểu đồ
        tempChart.update();
        humChart.update();

        // Tính chênh lệch nhiệt độ
      if (lastTemp !== null) {
        let diffTemp = (data.temp - lastTemp).toFixed(1);
        let tempChange = document.getElementById('tempChange');
        if (diffTemp > 0) {
          tempChange.innerHTML = `<p class="change up">↑ +${diffTemp} °C từ lần trước</p>`;
        } else if (diffTemp < 0) {
          tempChange.innerHTML = `<p class="change down">↓ ${diffTemp} °C từ lần trước</p>`;
        } else {
          tempChange.innerHTML = `<p class="change">Không đổi</p>`;
        }
      }
      lastTemp = data.temp;

      // Tính chênh lệch độ ẩm
      if (lastHum !== null) {
        let diffHum = (data.hum - lastHum).toFixed(1);
        let humChange = document.getElementById('humChange');
        if (diffHum > 0) {
          humChange.innerHTML = `<p class="change up">↑ +${diffHum} % từ lần trước</p>`;
        } else if (diffHum < 0) {
          humChange.innerHTML = `<p class="change down">↓ ${diffHum} % từ lần trước</p>`;
        } else {
          humChange.innerHTML = `<p class="change">Không đổi</p>`;
        }
      }
      lastHum = data.hum;

      // Polling TinyML predictions when switch is ON
      if (document.getElementById("switch").checked) {
          fetch('/tinyML?switch=1')
            .then(res => res.json())
            .then(tData => {
                if (tData.state === "on") {
                    document.getElementById("predict-state").innerText = tData.label;
                }
            })
            .catch(error => console.error("Lỗi khi load TinyML: ", error));
      }

      })
      .catch(error => {
        console.error("Lỗi khi lấy dữ liệu:", error);
      });
  }

  // Cập nhật mỗi 10 giây
  setInterval(updateSensors, 10000);
  updateSensors();

    // ===== LOGIC POPUP CẤU HÌNH WIFI & MQTT =====
  const modal = document.getElementById("configModal");
  const wifiIcon = document.getElementById("wifi-icon");
  const closeBtn = document.querySelector(".close-btn");
  const configForm = document.getElementById("configForm");

  if (wifiIcon) {
    wifiIcon.addEventListener("click", function() {
      modal.classList.add("show");
    });
  }

  if (closeBtn) {
    closeBtn.addEventListener("click", function() {
      modal.classList.remove("show");
    });
  }

  window.addEventListener("click", function(event) {
    if (event.target === modal) {
      modal.classList.remove("show");
    }
  });

  if (configForm) {
    configForm.addEventListener("submit", function(e) {
      e.preventDefault();
      
      const btn = document.querySelector(".btn-submit");
      const statusMsg = document.getElementById("statusMessage");
      
      btn.innerText = "Đang gửi dữ liệu...";
      btn.style.opacity = 0.7;
      btn.disabled = true;

      const ssid = encodeURIComponent(document.getElementById("ssid").value);
      const pass = encodeURIComponent(document.getElementById("pass").value);
      const server = encodeURIComponent(document.getElementById("server").value);
      const port = encodeURIComponent(document.getElementById("port").value);
      const token = encodeURIComponent(document.getElementById("token").value);

      const url = `/connect?ssid=${ssid}&pass=${pass}&server=${server}&port=${port}&token=${token}`;

      fetch(url)
        .then(response => response.text())
        .then(text => {
          statusMsg.style.display = "block";
          statusMsg.className = "success";
          statusMsg.innerHTML = "💾 " + text;
          btn.innerText = "Thành công!";
          
          setTimeout(() => {
            modal.classList.remove("show");
            btn.innerText = "Lưu & Khởi động lại mạng";
            btn.style.opacity = 1;
            btn.disabled = false;
            statusMsg.style.display = "none";
          }, 3000);
        })
        .catch(err => {
          statusMsg.style.display = "block";
          statusMsg.className = "error";
          statusMsg.innerHTML = "⚠️ Mất kết nối tới thiết bị (Có thể mạch đang Restart mạng).";
          btn.innerText = "Đã gửi lệnh";
          
          setTimeout(() => {
            btn.style.opacity = 1;
            btn.disabled = false;
          }, 3000);
        });
    });
  }

});


function loadStatus(){
  loadStatus1();
  loadStatus2();
}

// Hàm xử lý trạng thái led 1

// function loadStatus1() {
//   fetch('/status')
//     .then(response => response.json())
//     .then(data => {
//       let btn = document.getElementById("btn-led1");

//       if (data.led1 === 1) {
//         btn.innerText = "Tắt Đèn";
//         btn.classList.add("on");
//         btn.classList.remove("off");
//       } else {
//         btn.innerText = "Bật Đèn";
//         btn.classList.add("off");
//         btn.classList.remove("on");
//       }
//     })
//     .catch(error => console.error("Lỗi khi load trạng thái:", error));
// }

// // Hàm xử lý trạng thái led 2

// function loadStatus2() {
//   fetch('/status')
//     .then(response => response.json())
//     .then(data => {
//       let btn = document.getElementById("btn-led2");

//       if (data.led2 === 1) {
//         btn.innerText = "Tắt Đèn";
//         btn.classList.add("on");
//         btn.classList.remove("off");
//       } else {
//         btn.innerText = "Bật Đèn";
//         btn.classList.add("off");
//         btn.classList.remove("on");
//       }
//     })
//     .catch(error => console.error("Lỗi khi load trạng thái:", error));
// }

function loadStatus1() {
  fetch('/status')
    .then(response => response.json())
    .then(data => {
      if (data.led1 === 1) {
        document.getElementById("btn-led1").innerText = "Tắt Đèn";
        document.getElementById("led1Status").innerText = "LED1 ON";
      } else {
        document.getElementById("btn-led1").innerText = "Bật Đèn";
        document.getElementById("led1Status").innerText = "LED1 OFF";
      }
    })
    .catch(error => console.error("Lỗi khi load trạng thái:", error));
}

// Hàm xử lý trạng thái led 2
function loadStatus2() {
  fetch('/status')
    .then(response => response.json())
    .then(data => {
      if (data.led2 === 1) {
        document.getElementById("btn-led2").innerText = "Tắt Đèn";
        document.getElementById("led2Status").innerText = "LED2 ON";
      } else {
        document.getElementById("btn-led2").innerText = "Bật Đèn";
        document.getElementById("led2Status").innerText = "LED2 OFF";
      }
    })
    .catch(error => console.error("Lỗi khi load trạng thái:", error));
}

// Xử lý trạng thái load trang
window.onload = loadStatus;

// // Hàm xử lý nút led 1

// document.getElementById("btn-led1").addEventListener("click", function(){
//   let currentText = this.innerText;
//   let newState = currentText.includes("Bật") ? "on" : "off";

//   fetch('/led1?state=' + newState)
//     .then(response => response.text())
//     .then(data => {
//       loadStatus(); // giữ nguyên
//     })
//     .catch(error => console.error(error));
// });

// // Hàm xử lý nút led 2

// document.getElementById("btn-led2").addEventListener("click", function(){
//   let currentText = this.innerText;
//   let newState = currentText.includes("Bật") ? "on" : "off";

//   fetch('/led2?state=' + newState)
//     .then(response => response.text())
//     .then(data => {
//       loadStatus();
//     })
//     .catch(error => console.error(error));
// });

// ===== LOGIC ĐIỀU KHIỂN THIẾT BỊ =====
  document.getElementById("btn-led1").addEventListener("click", function(){
    let currentText = this.innerText;
    let newState = currentText.includes("Bật") ? "on" : "off";

    fetch('/led1?state=' + newState)
      .then(response => response.text())
      .then(data => {
        if (newState === "on") {
          this.innerText = "Tắt Đèn";
          this.classList.add("on");
          this.classList.remove("off");
        } else {
          this.innerText = "Bật Đèn";
          this.classList.add("off");
          this.classList.remove("on");
        }
      })
      .catch(error => console.error(error));
  });

  document.getElementById("btn-led2").addEventListener("click", function(){
    let currentText = this.innerText;
    let newState = currentText.includes("Bật") ? "on" : "off";

    fetch('/led2?state=' + newState)
      .then(response => response.text())
      .then(data => {
        if (newState === "on") {
          this.innerText = "Tắt Đèn";
          this.classList.add("on");
          this.classList.remove("off");
        } else {
          this.innerText = "Bật Đèn";
          this.classList.add("off");
          this.classList.remove("on");
        }
      })
      .catch(error => console.error(error));
  });


// Hàm xử lý nút tắt hết
// document.getElementById("btn-off").addEventListener("click", function(){
//   fetch('/off')
//     .then(response => response.text())
//     .then(data => {
//       console.log("ESP32 trả về:", data);
//       loadStatus();
//       document.getElementById("allStatus").innerText = data;
//     })
//     .catch(error => {
//       console.error("Lỗi khi gửi yêu cầu:", error);
//     });
// });

  document.getElementById("btn-off").addEventListener("click", function(){
    fetch('/off')
      .then(response => response.text())
      .then(data => {
        document.querySelectorAll("button").forEach(btn => {
          if (!btn.classList.contains("btn-off") && !btn.classList.contains("btn-refresh") && !btn.classList.contains("btn-submit")) { 
            btn.innerText = "Bật Đèn";
            btn.classList.add("off");
            btn.classList.remove("on");
          }
        });
      })
      .catch(error => console.error(error));
  });

// Hàm xử lý switch tinyML
document.getElementById("switch").addEventListener("change",function(){
  fetch('/tinyML?switch=' + (document.getElementById("switch").checked ? "1" : "0"))
    .then(response => response.json())
    .then(data => {
      if (data.state === "on") {
        document.getElementById("predict-state").innerText = data.label;
      } else {
        document.getElementById("predict-state").innerText = "--";
      }
    })
    .catch(error => console.error(error));
});



