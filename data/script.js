document.addEventListener("DOMContentLoaded", function () {

  const labels = ["1h", "2h", "3h", "4h", "5h"];
  const tempData = [27, 28, 29, 30, 29];
  const humData = [55, 57, 60, 58, 56];

  const tempCanvas = document.getElementById("tempChart");
  const humCanvas = document.getElementById("humChart");

  if (!tempCanvas || !humCanvas) {
    console.error("Không tìm thấy canvas!");
    return;
  }

  new Chart(tempCanvas, {
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
      maintainAspectRatio: false
    }
  });

  new Chart(humCanvas, {
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
      maintainAspectRatio: false
    }
  });

});

// Hàm cập nhật dữ liệu cảm biến
function updateSensors() {
  fetch('/sensors')
    .then(response => response.json())
    .then(data => {
      document.getElementById('temp').innerText = data.temp + " °C";
      document.getElementById('hum').innerText = data.hum + " %";
    })
    .catch(error => {
      console.error("Lỗi khi lấy dữ liệu:", error);
    });
}

setInterval(updateSensors, 5000);
updateSensors();

// Hàm xử lý nút led 1
document.getElementById("btn-led1").addEventListener("click", function(){
  let currentText = this.innerText;
  let newState = currentText.includes("Bật") ? "on" : "off";
  fetch('/led1?state=' + newState)
    .then(response => response.text())
    .then(data => {
      console.log("ESP32 trả về:", data);
      if (newState === "on") {
        this.innerText = "Tắt LED 1";
      } else {
        this.innerText = "Bật LED 1";
      }
    })
    .catch(error => {
      console.error("Lỗi khi gửi yêu cầu:", error);
    });
});

// Hàm xử lý nút led 2
document.getElementById("btn-led2").addEventListener("click", function(){
  let currentText = this.innerText;
  let newState = currentText.includes("Bật") ? "on" : "off";
  fetch('/led2?state=' + newState)
    .then(response => response.text())
    .then(data => {
      console.log("ESP32 trả về:", data);
      if (newState === "on") {
        this.innerText = "Tắt LED 2";
      } else {
        this.innerText = "Bật LED 2";
      }
    })
    .catch(error => {
      console.error("Lỗi khi gửi yêu cầu:", error);
    });
});

// Hàm xử lý nút tắt tất cả
document.getElementById("btn-off").addEventListener("click", function(){
  fetch('/all/off')
    .then(response => response.text())
    .then(data => {
      console.log("ESP32 trả về:", data);
      document.getElementById("btn-led1").innerText = "Bật LED 1";
      document.getElementById("btn-led2").innerText = "Bật LED 2";
    })
    .catch(error => {
      console.error("Lỗi khi gửi yêu cầu tắt tất cả:", error);
    });
});
