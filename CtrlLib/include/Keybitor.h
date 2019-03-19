

#ifdef __cplusplus
extern "C" {
#endif

  // check the presence of a device using the PS2KeyboardToggler driver filter
  // returns :  0 if no device is present
  //            1 if at least on device is present
  //            -1 on error
  int hasDevice();

  // executes a callback pssing each compatible device's path
  // returns :  0 if no device is present
  //            1 if at least on device is present
  //            -1 on error
  int foreachDevice(void(*)(const wchar_t*, void*), void*);

  // returns whether the provided device is toggle on or off
  // returns :  0 if the device is toggled off
  //            1 if the device is toggled on
  //            -1 on error
  int isDeviceEnable(const wchar_t*);

  // inverts the toggle state of the device and
  // returns whether it is toggle on or off
  // returns :  0 if the device is toggled off
  //            1 if the device is toggled on
  //            -1 on error
  int toggleDevice(const wchar_t*);

#ifdef __cplusplus
}
#endif
