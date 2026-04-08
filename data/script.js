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

      })
      .catch(error => {
        console.error("Lỗi khi lấy dữ liệu:", error);
      });
  }

  // Cập nhật mỗi 10 giây
  setInterval(updateSensors, 10000);
  updateSensors();
});


// Trạng thái mặc định ban đầu là tắt (class="off")
// Các nút được điều khiển offline qua UI và đồng bộ về server.

// Hàm xử lý nút led 1

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

// Hàm xử lý nút led 2

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

document.getElementById("btn-off").addEventListener("click", function(){
  fetch('/all/off')
    .then(response => response.text())
    .then(data => {
      document.querySelectorAll("button").forEach(btn => {
        if (!btn.classList.contains("btn-off") && !btn.classList.contains("btn-refresh")) { 
          btn.innerText = "Bật Đèn";
          btn.classList.add("off");
          btn.classList.remove("on");
        }
      });
    })
    .catch(error => console.error(error));
});