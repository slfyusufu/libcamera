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
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <errno.h>
#include <unistd.h>
#include <asm/types.h>         
#include <termios.h>
#include <linux/fb.h>
#include <sys/mman.h>
#include <sys/ioctl.h>
#include <time.h> // clock
#include <linux/videodev2.h>
//#include <mach/tccfb_ioctrl.h>

#include "camera.h"
//#include "include/myfb.h"



#define  ON 1
#define OFF 0

///////////////////////////////////
//for test
#include <mach/tcc_overlay_ioctl.h>
#include <mach/vioc_global.h>
#include "pmap.h"

#define OVERLAYDEVFILE	"/dev/overlay"

typedef struct tcc_overlay_handle tcc_overlay_handle_t;
struct tcc_overlay_handle {
	int						fd;
	overlay_video_buffer_t	overlay_info;
	pmap_t					pmap;
	void *					pdata;

    unsigned int			(*makepixel)(unsigned int a, unsigned int r, unsigned int g, unsigned int b);
    void					(*fill_rect)(void *pdata, int xres, unsigned int pixel,
										int xpos1, int ypos1, int xpos2, int ypos2);
};

#if 1
tcc_overlay_handle_t *delete_overlay_handle(tcc_overlay_handle_t *h)
{
    if (h) {
        if (h->pdata != MAP_FAILED) {
			munmap(h->pdata, h->pmap.size);
        }
		close(h->fd);
        free(h);
    }
    return NULL;
}
#endif
tcc_overlay_handle_t *new_overlay_handle(char *dev_name)
{
    int success = -1;
    tcc_overlay_handle_t *h = NULL;

    if (dev_name) {
        h = malloc(sizeof(tcc_overlay_handle_t));
        if (h) {
            memset(h, 0, sizeof(tcc_overlay_handle_t));
            h->fd = -1;
            h->pdata = MAP_FAILED;

            h->fd = open(dev_name, O_RDWR);
            if (h->fd != -1) {
				h->pmap.size = 0;
				pmap_get_info("overlay", &h->pmap); 

				if (h->pmap.size) {
	                h->pdata = mmap(0, h->pmap.size, PROT_READ|PROT_WRITE, MAP_SHARED, h->fd, h->pmap.base);
	                if (h->pdata != MAP_FAILED) {
                        h->makepixel = NULL;
                        h->fill_rect = NULL;
	                    success = 0;
	                } else 
	                    perror("[overlay_test] overlay mmap error !!!\n");
				}
            } else 
                perror("[overlay_test] Cannot open device !!!\n");
        } else 
            printf("[overlay_test] %s:%d Cannot allocate memory !!!\n", __func__, __LINE__);
    } else 
        printf("[overlay_test] %s:%d dev_name is NULL !!!\n", __func__, __LINE__);

    if (success != 0) {
        h = delete_overlay_handle(h);
    }
    return h;
}

/////////
#define camera_width 640
#define camera_height 480
int main(int argc, char *argv[])
{
	
#if 0
/////////////////////////framebuffer test////////////////////////////
    Framebuffer Myfb;
    init_framebuffer(&Myfb);


#endif
#if 1
//////////////////////////camera test////////////////////////////////
    CameraDevice cam_dev;
   
    open_cam(&cam_dev);
    if(cam_dev.fd == -1) exit(0);
    camera_querycap(&cam_dev);
    camera_enum_format(&cam_dev);
    //cam_dev.pix_format = V4L2_PIX_FMT_MJPEG;
    cam_dev.pix_format = V4L2_PIX_FMT_YUYV;
    //cam_dev.type = V4L2_BUF_TYPE_VIDEO_OUTPUT;
    cam_dev.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    camera_try_format(&cam_dev);
    camera_get_format(&cam_dev);
    camera_set_format(&cam_dev, camera_width, camera_height);
    camera_get_format(&cam_dev);
    enum_frame_sizes(&cam_dev);
    
    
    camera_get_parameters(&cam_dev);
    cam_dev.framerate.numerator = 1;
    cam_dev.framerate.denominator = 30;
    camera_set_parameters(&cam_dev);
    camera_get_parameters(&cam_dev);
    camera_get_crop(&cam_dev);
    
    cam_dev.memory_type = V4L2_MEMORY_MMAP;
    cam_dev.buffer_num = BUF_NUM;
    camera_request_buffer(&cam_dev);
    
	camera_query_buffer(&cam_dev);
    camera_queue_buffer(&cam_dev);
    camera_streamon(&cam_dev);
    
	camera_dequeue_buffer(&cam_dev);
	camera_queue_buffer(&cam_dev);

#if 0
    int i = 0;
    while(1){
        camera_dequeue_buffer(&cam_dev);
        for(i=0;i<cam_dev.buffer_num;i++)
            ;//memcpy(Myfb.fb_buf,cam_dev.cam_buffer[i],614400);
        camera_queue_buffer(&cam_dev);
    }
    
    camera_streamoff(&cam_dev);
    camera_release_buffer(&cam_dev);
#endif



#if 1 //for test
	tcc_overlay_handle_t *h_overlay = NULL;
	tcc_overlay_handle_t *hovr = NULL;

	overlay_video_buffer_t info;
	int xres, yres;
	void *pdata;
	unsigned int base[3] = {0, 0, 0};

	h_overlay = new_overlay_handle(OVERLAYDEVFILE);
    if (!h_overlay) {
        printf("Cannot allocate tcc_overlay_handle_t (#0) \n");
        goto _end;
    }
/////设置overlay显示位置，大小和格式，如果如要透明，设置为ARGB8888
	h_overlay->overlay_info.cfg.sx = 0;
	h_overlay->overlay_info.cfg.sy = 0;
	h_overlay->overlay_info.cfg.width = camera_width; //1024
	h_overlay->overlay_info.cfg.height = camera_height; //600
	h_overlay->overlay_info.cfg.format = VIOC_IMG_FMT_YUYV;
/////如果打开了CROP功能，需要设置如下参数，否则VIOC会出错
#ifdef CONFIG_OVERLAY_CROP
	h_overlay->overlay_info.cfg.crop_src.left = 0;
	h_overlay->overlay_info.cfg.crop_src.top = 0;
	h_overlay->overlay_info.cfg.crop_src.width = h_overlay->overlay_info.cfg.width;
	h_overlay->overlay_info.cfg.crop_src.height = h_overlay->overlay_info.cfg.height;
#endif
/////如果打开了SCALER功能，需要设置如下参数，否则VIOC会出错
#ifdef CONFIG_OVERLAY_SCALE
	h_overlay->overlay_info.cfg.dispwidth = h_overlay->overlay_info.cfg.width;
	h_overlay->overlay_info.cfg.dispheight = h_overlay->overlay_info.cfg.height;
	ioctl(h_overlay->fd, OVERLAY_SET_CONFIGURE,&h_overlay->overlay_info.cfg);
#endif

///////设置层次 
	unsigned int ovp;
	ovp = 0 ;   //ovp 3-0-1-2  overlay is RDMA3 now
	ioctl(h_overlay->fd, OVERLAY_SET_WMIXER_OVP, &ovp);
#ifndef USE_PMAP
//////把overlay内存映射到overlay层上
	h_overlay->overlay_info.addr = h_overlay->pmap.base;
#ifdef CONFIG_OVERLAY_CROP	
	h_overlay->overlay_info.addr1 = h_overlay->pmap.base;
	h_overlay->overlay_info.addr2 = h_overlay->pmap.base;
#endif
	ioctl(h_overlay->fd, OVERLAY_PUSH_VIDEO_BUFFER,&h_overlay->overlay_info);
#endif

/////对overlay内存进行操作	
	hovr = h_overlay;
	pdata = hovr->pdata;
	xres  = h_overlay->overlay_info.cfg.width;
	yres  = h_overlay->overlay_info.cfg.height;
	
	//memset(pdata, 0x0, (xres*yres*2));

    int i = 0;
    int num=0;
    clock_t clock_end;
	clock_t clock_start = clock();
    while(1){
		num++;
        camera_dequeue_buffer(&cam_dev);
        clock_end = clock();
        if (((double)(clock_end - clock_start)/1000) >= 1)
		{
			printf("Num = %d.\n",num);
			clock_start = clock_end;
			num = 0;
		}
#if 1        
        for(i=0;i<cam_dev.buffer_num;i++)
        {
#ifndef USE_PMAP
            memcpy(pdata,cam_dev.cam_buffer[i],153600);
#endif
#ifdef USE_PMAP
			h_overlay->overlay_info.addr = cam_dev.cam_buffer[i];
			#ifdef CONFIG_OVERLAY_CROP	
			h_overlay->overlay_info.addr1 = h_overlay->overlay_info.addr;
			h_overlay->overlay_info.addr2 = h_overlay->overlay_info.addr;
			#endif
			ioctl(h_overlay->fd, OVERLAY_PUSH_VIDEO_BUFFER,&h_overlay->overlay_info);
#endif
			printf("address = 0x%x;\n",cam_dev.cam_buffer[i]);
		}
#endif
        camera_queue_buffer(&cam_dev);
    }

_end:
	delete_overlay_handle(h_overlay);
	
    camera_streamoff(&cam_dev);
    camera_release_buffer(&cam_dev);

#endif
    close(cam_dev.fd);
    
    #if 0
    init_camera
    set_camera_parm
    get_frame
    start_preview
    start_capture
    stop_preview
    release_camera
    #endif
///////////////////////////////////////////////////////////////////////////////

#endif

    //deinit_framebuffer(&iMyfb);

	return 0;
}

