#include "tinyml.h"

// Globals, for the convenience of one-shot setup.
namespace
{
    tflite::ErrorReporter *error_reporter = nullptr;
    const tflite::Model *model = nullptr;
    tflite::MicroInterpreter *interpreter = nullptr;
    TfLiteTensor *input = nullptr;
    TfLiteTensor *output = nullptr;
    constexpr int kTensorArenaSize = 8 * 1024; // Adjust size based on your model
    uint8_t tensor_arena[kTensorArenaSize];
} // namespace

void setupTinyML()
{
    Serial.println("TensorFlow Lite Init....");
    static tflite::MicroErrorReporter micro_error_reporter;
    error_reporter = &micro_error_reporter;

    model = tflite::GetModel(dht_anomaly_model_tflite); // g_model_data is from model_data.h
    if (model->version() != TFLITE_SCHEMA_VERSION)
    {
        error_reporter->Report("Model provided is schema version %d, not equal to supported version %d.",
                               model->version(), TFLITE_SCHEMA_VERSION);
        return;
    }

    static tflite::AllOpsResolver resolver;
    static tflite::MicroInterpreter static_interpreter(
        model, resolver, tensor_arena, kTensorArenaSize, error_reporter);
    interpreter = &static_interpreter;

    TfLiteStatus allocate_status = interpreter->AllocateTensors();
    if (allocate_status != kTfLiteOk)
    {
        error_reporter->Report("AllocateTensors() failed");
        return;
    }

    input = interpreter->input(0);
    output = interpreter->output(0);

    Serial.println("TensorFlow Lite Micro initialized on ESP32.");
}

void tiny_ml_task(void *pvParameters)
{
    SystemHandles* handles = (SystemHandles*)pvParameters;
    SensorData data;
    TinyMLData predict_data;
    String last_predict_state = "";
    setupTinyML();
    
    for (;;) {
        // Chờ tín hiệu từ queue trigger 
        int trigger;
        if (xQueueReceive(handles->qTrigger, &trigger, portMAX_DELAY) == pdTRUE) {
            // Lấy dữ liệu sensor
            if (xQueuePeek(handles->qLcd, &data, 0) == pdTRUE) {
                input->data.f[0] = data.temperature;
                input->data.f[1] = data.humidity;
            }

            // Chạy inference
            TfLiteStatus status = interpreter->Invoke();
            if (status == kTfLiteOk) {
                float result = output->data.f[0];
                if (result > 0.5) {
                    predict_data.predict_state = "CRITICAL";
                } else {
                    predict_data.predict_state = "NORMAL";
                }
                predict_data.predict_value = result;

                // Gửi kết quả vào queue TinyML
                xQueueOverwrite(handles->qTinyML, &predict_data);

                xSemaphoreTake(handles->mutexDeviceState, portMAX_DELAY);
                bool is_tinyml = handles->deviceState.tinyml_mode;
                xSemaphoreGive(handles->mutexDeviceState);

                if (is_tinyml) {
                    if (predict_data.predict_state != last_predict_state) {
                        xSemaphoreGive(handles->semLcd);
                        last_predict_state = predict_data.predict_state;
                    }
                }
            }
        }
    }
}