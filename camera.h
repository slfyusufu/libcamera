//********************************************************************************************
/**
 * @file        
 * @brief		
 *
 * @author      Yusuf.Sha, Telechips Shenzhen Rep.
 * @date        2016/11/08
 */
//********************************************************************************************
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <asm/types.h>
#include <errno.h>
#include <unistd.h>
#include <termios.h>
#include <linux/fb.h>
#include <sys/mman.h>
#include <sys/ioctl.h>
#include <linux/videodev2.h>
//#include "include/videodev2.h"
#include <autoconf.h>

#define cam_debug  1
#if cam_debug
#define CAM_DBG(x...) printf("[Camera] " x)
#else 
#define CAM_DBG(x...)
#endif

#define CAM_DEV "/dev/video1"
#define BUF_NUM 8

#ifdef CONFIG_UVC_USE_TCC_PMAP
#define USE_PMAP
#endif

typedef struct {
    char                        dev_name[12];
    int 						fd;
    
    struct v4l2_capability      vcap;
    struct v4l2_fmtdesc         fmtdesc;   
    struct v4l2_format          format;
    struct v4l2_frmsizeenum     fsize;
    struct v4l2_frmivalenum     fival;
    struct v4l2_streamparm      para;
    struct v4l2_fract           framerate;
    struct v4l2_cropcap         cropcap;
    struct v4l2_requestbuffers  reqbufs;
    
    enum v4l2_buf_type          type;
    enum v4l2_memory            memory_type;
    __u32                       pix_format;
    __u32                       dispwidth;
    __u32                       dispheight;
    __u32                       imagesize;
    __u32                       buffer_num;
    void                        *cam_buffer[BUF_NUM];
    //unsigned char				*buffers[NUM_BUF];
    
	int							preview_width;
	int							preview_height;
	unsigned int				preview_fmt;
	//camera_mode					cam_mode;
	
	//pthread_t frame_threads;

}CameraDevice,*pCameraDevice;

//extern int cam_fd;

int open_cam(CameraDevice *cam);
int camera_querycap(CameraDevice *cam);
int camera_enum_format(CameraDevice *cam);
int camera_try_format(CameraDevice *cam);
int camera_get_format(CameraDevice *cam);
int camera_set_format(CameraDevice *cam, __u32 width, __u32 height);
int enum_frame_sizes(CameraDevice *cam);
int enum_frame_intervals(CameraDevice *cam); 

int camera_get_parameters(CameraDevice *cam);
int camera_set_parameters(CameraDevice *cam);
int camera_get_crop(CameraDevice *cam);
int camera_request_buffer(CameraDevice *cam);
int camera_query_buffer(CameraDevice *cam);
int camera_queue_buffer(CameraDevice *cam);
int camera_dequeue_buffer(CameraDevice *cam);
int camera_streamon(CameraDevice *cam);
int camera_streamoff(CameraDevice *cam);
int camera_release_buffer(CameraDevice *cam);

