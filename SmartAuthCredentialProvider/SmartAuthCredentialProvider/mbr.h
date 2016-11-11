
// Copyright (C)2015 MtoM

// 인터페이스를 등록할때 필요한 드라이버 고유의 GUID를 정의한다.
DEFINE_GUID(MTOM_GUID, 0x873fdf35, 0x61a8, 0x11d1, 0xaa, 0x5e, 0x0, 0xc0, 0x4f, 0xb1, 0x72, 0x8b);

//DeviceIoControl 함수에서 사용할 제어 코드 정의
//
#define IOCTL_MTOM_INDEX  0x0800

#define IOCTL_GET_CONFIGURATION_DESCRIPTOR  \
            CTL_CODE(FILE_DEVICE_UNKNOWN, \
                IOCTL_MTOM_INDEX+0,\
                METHOD_BUFFERED,  \
                FILE_ANY_ACCESS)

#define IOCTL_SEND_CLASS_REQUEST \
            CTL_CODE(FILE_DEVICE_UNKNOWN, \
                IOCTL_MTOM_INDEX+1,\
                METHOD_BUFFERED,  \
                FILE_ANY_ACCESS)

#define IOCTL_RESET_PIPE \
            CTL_CODE(FILE_DEVICE_UNKNOWN, \
                IOCTL_MTOM_INDEX+2,\
                METHOD_BUFFERED,  \
                FILE_ANY_ACCESS)
