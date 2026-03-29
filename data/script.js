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

// Hàm xử lý nút nguồn
document.getElementById("btn-power").addEventListener("click", function() {
  let currentText = this.innerText;
  let newState = currentText.includes("Bật") ? "on" : "off";

  fetch('/power?state=' + newState)
    .then(response => response.text())
    .then(data => {
      console.log("ESP32 trả về:", data);
      if (newState === "on") {
        this.innerText = "Tắt Nguồn";
      } else {
        this.innerText = "Bật Nguồn";
      }
    })
    .catch(error => {
      console.error("Lỗi khi gửi yêu cầu:", error);
    });
});

// Hàm xử lý nút led 1
document.getElementById("btn-led").addEventListener("click", function(){
  let currentText = this.innerText;
  let newState = currentText.includes("Bật") ? "on" : "off";
  fetch('/led?state=' + newState)
    .then(response => response.text())
    .then(data => {
      console.log("ESP32 trả về:", data);
      if (newState === "on") {
        this.innerText = "Tắt Đèn";
      } else {
        this.innerText = "Bật Đèn";
      }
    })
    .catch(error => {
      console.error("Lỗi khi gửi yêu cầu:", error);
    });
});


// Hàm xử lý nút fan 1
document.getElementById("btn-fan").addEventListener("click", function(){
  let currentText = this.innerText;
  let newState = currentText.includes("Bật") ? "on" : "off";
  fetch('/fan?state=' + newState)
    .then(response => response.text())
    .then(data => {
      console.log("ESP32 trả về:", data);
      if (newState === "on") {
        this.innerText = "Tắt Quạt";
      } else {
        this.innerText = "Bật Quạt";
      }
    })
    .catch(error => {
      console.error("Lỗi khi gửi yêu cầu:", error);
    });
});
