#ifndef _PUBLIC_H
#define _PUBLIC_H

#define IOCTL_INDEX             0x800

#define IOCTL_PS2KBDTGL_TOGGLE_DISCARD CTL_CODE( FILE_DEVICE_KEYBOARD,   \
                                                        IOCTL_INDEX,    \
                                                        METHOD_BUFFERED,    \
                                                        FILE_READ_DATA)

#define IOCTL_PS2KBDTGL_GET_STATE CTL_CODE( FILE_DEVICE_KEYBOARD,   \
                                                        IOCTL_INDEX+1,    \
                                                        METHOD_BUFFERED,    \
                                                        FILE_READ_DATA)


#endif
