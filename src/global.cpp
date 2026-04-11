#include "global.h"

// The only global needed for passing signals maybe, or nothing if handled by tasks/events
// But since xBinarySemaphoreInternet is used in legacy code (if any), I'll keep it or remove it entirely.
// I will actually remove xBinarySemaphoreInternet since we use WiFiEvents directly now and there is no while loop polling it!