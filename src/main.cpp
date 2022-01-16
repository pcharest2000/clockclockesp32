#include "config.h"
#ifndef SIM
#include "fabgl.h"
#include <Arduino.h>
#else
#include "Canvas.h"
#endif
#include "Animation.h"
#include "ClockMatrix.h"
#include "clock.h"
#include "myTime.h"
#include <cstdint>

#include "dali_digit.h"
void updateClockTask(void *pvParameters);
TaskHandle_t updateTask;

fabgl::VGA2Controller displayController;
Canvas cv(&displayController);
ClockMatrix clocks;
DaliDigit dalDig;
#ifdef GETTIME
myTime myTi;
#endif

void updateClockTask(void *pvParameters) {
  for (;;) {
    //   if (myTi.getTimeChanged())
    clockLoop(&clocks, &myTi);
  }
}

void setup() {
  Serial.begin(115200);
#ifdef GETTIME
  myTi.updateNptTime();
#endif
  displayController.begin(GPIO_NUM_22, GPIO_NUM_19, GPIO_NUM_5, GPIO_NUM_23,
                          GPIO_NUM_15);
  displayController.setResolution(SVGA_800x600_60Hz, -1, -1, true);
  clocks.begin(&cv);
  dalDig.begin(&cv, &myTi);
  xTaskCreatePinnedToCore(
      updateClockTask, /* Task function. */
      "updateTask",    /* name of task. */
      10000,           /* Stack size of task */
      NULL,            /* parameter of the task */
      0,               /* priority of the task */
      &updateTask,     /* Task handle to keep track of created task */
      0);              /* pin task to core 0 */
  delay(1);
  clocks.setSpeedAll(18, 18);
}
void loop() {
  cv.clear();

#ifdef SHOWFPS
  static int64_t stime = esp_timer_get_time();
  static int FPS = 0;
  static int FPSCounter = 0;
  if (esp_timer_get_time() - stime > 1000000) {
    // calculate FPS
    FPS = FPSCounter;
    stime = esp_timer_get_time();
    FPSCounter = 0;
  }
  ++FPSCounter;
  cv.drawTextFmt(0, 0, "%d FPS ", FPS);
  cv.drawTextFmt(0, 30, "Free Memory: %d bytes",
                 heap_caps_get_free_size(MALLOC_CAP_32BIT));
#endif

  // clocks.draw();
  dalDig.drawClockBlend();
  cv.swapBuffers();
  // #ifdef GETTIME
  //   myTi.getTime();
  //   // myTi.printInfo();
  //   // Serial.print("What changed: ");
  //   // Serial.println(state);
  //   // delay(1000);
  // #endif
}
