//********************************************************************************************
/**
 * @file        
 * @brief		
 *
 * @author      Yusuf.Sha, Telechips Shenzhen Rep.
 * @date        2016/11/08
 */
//********************************************************************************************
#include "camera.h"

//int fd = 0;



int open_cam(CameraDevice *cam)
{
    //open camera device
    //int fd=-1;
    sprintf(cam->dev_name, CAM_DEV);
	cam->fd = open(cam->dev_name, O_RDONLY);	//rd&wr
	if(cam->fd<0) {
		fprintf(stderr, "Open camera fail!\n");
		close(cam->fd);
		return -1;
	} else
		fprintf(stdout, "Open camera success!\n");
	return cam->fd;
}

///////////////////////////////////////////////////////////////////////////////VIDIOC_QUERYCAP


int camera_querycap(CameraDevice *cam)
{
	//struct v4l2_capability vcap;
	int ret = -1;
	
	memset(&cam->vcap, 0, sizeof(struct v4l2_capability)); 
	if((ret = ioctl(cam->fd, VIDIOC_QUERYCAP, &(cam->vcap)))<0)
	{
		CAM_DBG("VIDIOCGCAP fail\n");
		return -EINVAL;
	}
	else
        CAM_DBG( "driver=%s, card=%s, bus=%s, "
                "version=0x%08x, "
                "capabilities=0x%08x\n",
                cam->vcap.driver, cam->vcap.card, cam->vcap.bus_info,
                cam->vcap.version,
                cam->vcap.capabilities);
    return ret;
}

int camera_enum_format(CameraDevice *cam)
{
///////////////////////////////////////////////////////////////////////////////VIDIOC_ENUM_FMT
// 419 struct v4l2_fmtdesc {
// 420     __u32           index;             /* Format number      */
// 421     enum v4l2_buf_type  type;              /* buffer type        */
// 422     __u32               flags;
// 423     __u8            description[32];   /* Description string */
// 424     __u32           pixelformat;       /* Format fourcc      */
// 425     __u32           reserved[4];
// 426 };
    int ret = -1;
    //struct v4l2_fmtdesc fmtdesc;
    memset(&cam->fmtdesc, 0, sizeof(struct v4l2_fmtdesc));
    cam->fmtdesc.index = 0;
    cam->fmtdesc.type = cam->type;//V4L2_BUF_TYPE_VIDEO_CAPTURE;//V4L2_BUF_TYPE_VIDEO_OUTPUT;
    while ((ret = ioctl(cam->fd, VIDIOC_ENUM_FMT, &(cam->fmtdesc))) == 0)
    {
            cam->fmtdesc.index++;
            CAM_DBG("{ pixelformat = '%c%c%c%c', description = '%s' }\n",
                       cam->fmtdesc.pixelformat & 0xFF,
                      (cam->fmtdesc.pixelformat >> 8) & 0xFF, 
                      (cam->fmtdesc.pixelformat >> 16) & 0xFF,
                      (cam->fmtdesc.pixelformat >> 24) & 0xFF, 
                      cam->fmtdesc.description);
    }
    return 0;
}

///////////////////////////////////////////////////////////////////////////////VIDIOC_TRY_FMT
 //277 /*
 //278  *  V I D E O   I M A G E   F O R M A T
 //279  */
 //280 struct v4l2_pix_format {
 //281     __u32           width;
 //282     __u32           height;
 //283     __u32           pixelformat;
 //284     enum v4l2_field field;
 //285     __u32           bytesperline;   /* for padding, zero if unused */
 //286     __u32           sizeimage;
 //287     enum v4l2_colorspace    colorspace;
 //288     __u32           priv;       /* private data, depends on pixelformat */
 //289 };

//1978 struct v4l2_format {
//1979     enum v4l2_buf_type type;       //V4L2_BUF_TYPE_VIDEO_CAPTURE
//1980     union {
//1981         struct v4l2_pix_format      pix;     /* V4L2_BUF_TYPE_VIDEO_CAPTURE */
//1982         struct v4l2_pix_format_mplane   pix_mp;  /* V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE */
//1983         struct v4l2_window      win;     /* V4L2_BUF_TYPE_VIDEO_OVERLAY */
//1984         struct v4l2_vbi_format      vbi;     /* V4L2_BUF_TYPE_VBI_CAPTURE */
//1985         struct v4l2_sliced_vbi_format   sliced;  /* V4L2_BUF_TYPE_SLICED_VBI_CAPTURE */
//1986         __u8    raw_data[200];                   /* user-defined */
//1987     } fmt;
//1988 };
int camera_try_format(CameraDevice *cam)
{
    int ret = -1;
    //struct v4l2_format format;
    memset(&cam->format, 0, sizeof(struct v4l2_format));
    cam->format.type = cam->type;//V4L2_BUF_TYPE_VIDEO_CAPTURE;//V4L2_BUF_TYPE_VIDEO_OUTPUT;
    cam->format.fmt.pix.pixelformat = cam->pix_format;//V4L2_PIX_FMT_YUYV;//V4L2_PIX_FMT_MJPEG
    if((ret = ioctl(cam->fd, VIDIOC_TRY_FMT, &cam->format)) < 0)
        CAM_DBG("This camera can not support this type of format.\n");
    else
        CAM_DBG("Congratulations!! This camera can support this format.\n");
    
    return 0;
}

///////////////////////////////////////////////////////////////////////////////VIDIOC_G_FMT
int camera_get_format(CameraDevice *cam)  
{  
    //struct v4l2_format fmt;  
    int ret = -1;  
      
    cam->format.type = cam->type;//V4L2_BUF_TYPE_VIDEO_CAPTURE;//V4L2_BUF_TYPE_VIDEO_OUTPUT;  
    ret = ioctl(cam->fd, VIDIOC_G_FMT, &cam->format);  
    if(ret<0){  
        CAM_DBG("Get Format failed\n");
        return ret;
    }  
    cam->dispheight = cam->format.fmt.pix.height;  
    cam->dispwidth = cam->format.fmt.pix.width;  
    cam->imagesize = cam->format.fmt.pix.sizeimage;  
    
    CAM_DBG("width = %u, height = %u, sizeimage = %u.\n",cam->dispwidth,cam->dispheight,cam->imagesize);
    
    return ret;
}
///////////////////////////////////////////////////////////////////////////////VIDIOC_S_FMT
int camera_set_format(CameraDevice *cam, __u32 width, __u32 height)  
{  
    //struct v4l2_format fmt;
    int ret = -1;

    cam->format.type = cam->type;
    ret = ioctl(cam->fd, VIDIOC_G_FMT, &cam->format);
    if(ret<0) {
        printf("Set Format failed\n");
        return ret;
    }
    cam->format.fmt.pix.width = width;
    cam->format.fmt.pix.height = height;
    cam->format.fmt.pix.pixelformat = cam->pix_format;//V4L2_PIX_FMT_YUYV;//V4L2_PIX_FMT_YUYV V4L2_PIX_FMT_RGB565
    ret = ioctl(cam->fd, VIDIOC_S_FMT, &cam->format);
    if(ret<0) {
        CAM_DBG("Set Format failed\n");
        return ret;  
    }
    CAM_DBG("Set Format successful!\n");
    return 0;
} 

///////////////////////////////////////////////////////////////////////////////VIDIOC_ENUM_FRAMESIZES
// 456 struct v4l2_frmsizeenum {
// 457     __u32           index;      /* Frame size number */
// 458     __u32           pixel_format;   /* Pixel format */
// 459     __u32           type;       /* Frame size type the device supports. */
// 460 
// 461     union {                 /* Frame size */
// 462         struct v4l2_frmsize_discrete    discrete;
// 463         struct v4l2_frmsize_stepwise    stepwise;
// 464     };
// 465 
// 466     __u32   reserved[2];            /* Reserved space for future use */
// 467 };
// 442 struct v4l2_frmsize_discrete {
// 443     __u32           width;      /* Frame width [pixel] */
// 444     __u32           height;     /* Frame height [pixel] */
// 445 };
// 446 
// 447 struct v4l2_frmsize_stepwise {
// 448     __u32           min_width;  /* Minimum frame width [pixel] */
// 449     __u32           max_width;  /* Maximum frame width [pixel] */
// 450     __u32           step_width; /* Frame width step size [pixel] */
// 451     __u32           min_height; /* Minimum frame height [pixel] */
// 452     __u32           max_height; /* Maximum frame height [pixel] */
// 453     __u32           step_height;    /* Frame height step size [pixel] */
// 454 };

int enum_frame_sizes(CameraDevice *cam)  
{  
    int ret = 0;  
    //struct v4l2_frmsizeenum fsize;  
      
    memset(&cam->fsize, 0, sizeof(struct v4l2_frmsizeenum));  
    cam->fsize.index = 0;  
    cam->fsize.pixel_format = cam->pix_format;  
    while ((ret = ioctl(cam->fd, VIDIOC_ENUM_FRAMESIZES, &cam->fsize)) == 0) { 
        if (cam->fsize.type == V4L2_FRMSIZE_TYPE_DISCRETE) {  
            CAM_DBG("[%u].{ discrete: width = %u, height = %u }\n",cam->fsize.index,  
                   cam->fsize.discrete.width, cam->fsize.discrete.height);  
            ret = enum_frame_intervals(cam);  
            if (ret != 0)  
                printf(" Unable to enumerate frame sizes.\n");  
        } else if (cam->fsize.type == V4L2_FRMSIZE_TYPE_CONTINUOUS) {  
            CAM_DBG("{ continuous: min { width = %u, height = %u } .. "  
                   "max { width = %u, height = %u } }\n",  
                   cam->fsize.stepwise.min_width, cam->fsize.stepwise.min_height,  
                   cam->fsize.stepwise.max_width, cam->fsize.stepwise.max_height);  
            CAM_DBG("  Refusing to enumerate frame intervals.\n");  
            break;  
        } else if (cam->fsize.type == V4L2_FRMSIZE_TYPE_STEPWISE) {  
            CAM_DBG("{ stepwise: min { width = %u, height = %u } .. "  
                   "max { width = %u, height = %u } / "  
                   "stepsize { width = %u, height = %u } }\n",  
                   cam->fsize.stepwise.min_width, cam->fsize.stepwise.min_height,  
                   cam->fsize.stepwise.max_width, cam->fsize.stepwise.max_height,  
                   cam->fsize.stepwise.step_width, cam->fsize.stepwise.step_height);  
            CAM_DBG("  Refusing to enumerate frame intervals.\n");  
            break;  
        }  
        cam->fsize.index++;  
    } 
    if (ret != 0 && errno != EINVAL) {
        CAM_DBG("ERROR enumerating frame sizes...\n");  
        return errno;  
    }  
      
    return 0;  
}  
/////////////////////////////////////////////////////////////////////////////////VIDIOC_ENUM_FRAMEINTERVALS
//484 struct v4l2_frmivalenum {
//485     __u32           index;      /* Frame format index */
//486     __u32           pixel_format;   /* Pixel format */
//487     __u32           width;      /* Frame width */
//488     __u32           height;     /* Frame height */
//489     __u32           type;       /* Frame interval type the device supports. */
//490 
//491     union {                 /* Frame interval */
//492         struct v4l2_fract       discrete;
//493         struct v4l2_frmival_stepwise    stepwise;
//494     };
//495 
//496     __u32   reserved[2];            /* Reserved space for future use */
//497 };
//498 #endif

//478 struct v4l2_frmival_stepwise {
//479     struct v4l2_fract   min;        /* Minimum frame interval [s] */
//480     struct v4l2_fract   max;        /* Maximum frame interval [s] */
//481     struct v4l2_fract   step;       /* Frame interval step size [s] */
//482 };
int enum_frame_intervals(CameraDevice *cam)  
{  
    int ret = -1;  
    //struct v4l2_frmivalenum fival;  
      
    memset(&cam->fival, 0, sizeof(struct v4l2_frmivalenum));  
    cam->fival.index = 0;  
    cam->fival.pixel_format = cam->fsize.pixel_format;  
    cam->fival.width = cam->fsize.discrete.width;  
    cam->fival.height = cam->fsize.discrete.height;  
    CAM_DBG("\tTime interval between frame: ");  
    while ((ret = ioctl(cam->fd, VIDIOC_ENUM_FRAMEINTERVALS, &cam->fival)) == 0) {  
        if (cam->fival.type == V4L2_FRMIVAL_TYPE_DISCRETE) {  
            CAM_DBG("%u/%u, ",  
                   cam->fival.discrete.numerator, cam->fival.discrete.denominator);  
        } else if (cam->fival.type == V4L2_FRMIVAL_TYPE_CONTINUOUS) {  
            CAM_DBG("{min { %u/%u } .. max { %u/%u } }, ",  
                   cam->fival.stepwise.min.numerator, cam->fival.stepwise.min.numerator,  
                   cam->fival.stepwise.max.denominator, cam->fival.stepwise.max.denominator);  
            break;  
        } else if (cam->fival.type == V4L2_FRMIVAL_TYPE_STEPWISE) {  
            CAM_DBG("{min { %u/%u } .. max { %u/%u } / "  
                   "stepsize { %u/%u } }, ",  
                   cam->fival.stepwise.min.numerator, cam->fival.stepwise.min.denominator,  
                   cam->fival.stepwise.max.numerator, cam->fival.stepwise.max.denominator,  
                   cam->fival.stepwise.step.numerator, cam->fival.stepwise.step.denominator);  
            break;  
        }  
        cam->fival.index++;  
    }  
    CAM_DBG("\n");  
    if (ret != 0 && errno != EINVAL) {  
        CAM_DBG("ERROR enumerating frame intervals");  
        return errno;  
    }  
      
    return 0;  
}  


/////////////////////////////////////////////////////////////////////////////////VIDIOC_G_PARM
//1990 /*  Stream type-dependent parameters
//1991  */
//1992 struct v4l2_streamparm {
//1993     enum v4l2_buf_type type;
//1994     union {
//1995         struct v4l2_captureparm capture;
//1996         struct v4l2_outputparm  output;
//1997         __u8    raw_data[200];  /* user-defined */
//1998     } parm;
//1999 };

//704 struct v4l2_captureparm {
//705     __u32          capability;    /*  Supported modes */
//706     __u32          capturemode;   /*  Current mode */
//707     struct v4l2_fract  timeperframe;  /*  Time per frame in .1us units */
//708     __u32          extendedmode;  /*  Driver-specific extensions */
//709     __u32          readbuffers;   /*  # of buffers for read */
//710     __u32          reserved[4];
//711 };
//712 
//
//233 struct v4l2_fract {
//234     __u32   numerator;
//235     __u32   denominator;
//236 };
//疑问：这里的capability指的是什么?????????????????????????

int camera_get_parameters(CameraDevice *cam)
{
    int ret = -1;
    
    memset(&cam->para, 0, sizeof(struct v4l2_streamparm)); 
    cam->para.type = cam->type;
    ret = ioctl(cam->fd, VIDIOC_G_PARM, &cam->para);  
    if(ret<0){  
        CAM_DBG("Get Pamameters failed!\n");
        return ret;
    }
    cam->framerate.numerator = cam->para.parm.capture.timeperframe.numerator;
    cam->framerate.denominator = cam->para.parm.capture.timeperframe.denominator;
    CAM_DBG("\n\tcapability: %u;\n"
            "\tcapturemode:  %u;\n"
            "\textendedmode: %u;\n"
            "\treadbuffers:  %u;\n "
            "\tnumerator:    %u;\n "
            "\tdenminator:   %u;\n",
            cam->para.parm.capture.capability,  cam->para.parm.capture.capturemode,
            cam->para.parm.capture.extendedmode,cam->para.parm.capture.readbuffers,
            cam->para.parm.capture.timeperframe.numerator,
            cam->para.parm.capture.timeperframe.denominator);
    //CAM_DBG("numerator = %u, denominator = %u.\n",cam->framerate.numerator,cam->framerate.denominator);

    return 0;
}
/////////////////////////////////////////////////////////////////////////////////VIDIOC_S_PARM
int camera_set_parameters(CameraDevice *cam)
{
    int ret = -1;
    
    memset(&cam->para, 0, sizeof(struct v4l2_streamparm)); 
    cam->para.type = cam->type;
    cam->para.parm.capture.timeperframe.numerator = cam->framerate.numerator;
    cam->para.parm.capture.timeperframe.denominator = cam->framerate.denominator;
    ret = ioctl(cam->fd, VIDIOC_S_PARM, &cam->para);  
    if(ret<0){  
        CAM_DBG("Set Pamameters failed\n");
        return ret;
    }else
        CAM_DBG("Set parameters Successful.\n");
    
    return 0;
}


/////////////////////////////////////////////////////////////////////////////////VIDIOC_CROPCAP
//729 struct v4l2_cropcap {
//730     enum v4l2_buf_type      type;
//731     struct v4l2_rect        bounds;
//732     struct v4l2_rect        defrect;
//733     struct v4l2_fract       pixelaspect;
//734 };
//
//
//226 struct v4l2_rect {
//227     __s32   left;
//228     __s32   top;
//229     __s32   width;
//230     __s32   height;
//231 };

int camera_get_crop(CameraDevice *cam)
{
    int ret;
    
    memset(&cam->cropcap, 0, sizeof(struct v4l2_cropcap)); 
    cam->cropcap.type = cam->type;

    ret = ioctl(cam->fd, VIDIOC_CROPCAP, &cam->cropcap);  
    if(ret<0){  
        CAM_DBG("Get Crop failed\n");
        return ret;
    }else
        CAM_DBG("Get Crop Successful.\n");
    
    CAM_DBG("Bounds : left = %d, top = %d, width = %d, height = %d\n",
             cam->cropcap.bounds.left, cam->cropcap.bounds.top,
             cam->cropcap.bounds.width,cam->cropcap.bounds.height);
    CAM_DBG("Defrect: left = %d, top = %d, width = %d, height = %d\n",
             cam->cropcap.defrect.left, cam->cropcap.defrect.top,
             cam->cropcap.defrect.width,cam->cropcap.defrect.height);    
    return 0;    

}

/////////////////////////////////////////////////////////////////////////////////VIDIOC_REQBUFS
//574 struct v4l2_requestbuffers {
//575     __u32           count;
//576     enum v4l2_buf_type      type;
//577     enum v4l2_memory        memory;
//578     __u32           reserved[2];
//579 };
//
//184 enum v4l2_memory {
//185     V4L2_MEMORY_MMAP             = 1,
//186     V4L2_MEMORY_USERPTR          = 2,
//187     V4L2_MEMORY_OVERLAY          = 3,
//188 };

int camera_request_buffer(CameraDevice *cam)
{
    int ret = -1;
    //struct v4l2_requestbuffers reqbufs;
    
    memset(&cam->reqbufs, 0, sizeof(struct v4l2_requestbuffers));
    cam->reqbufs.count = cam->buffer_num;
    cam->reqbufs.type = cam->type;
    cam->reqbufs.memory = cam->memory_type;
    
    ret = ioctl(cam->fd, VIDIOC_REQBUFS, &cam->reqbufs); 
    if(ret<0){  
        CAM_DBG("Request buffer failed\n");
        return ret;
    }else
        CAM_DBG("Request buffer Successful. ret = %u.\n", ret);
        
    return 0;
}

/////////////////////////////////////////////////////////////////////////////////VIDIOC_QUERYBUF    &&   VIDIOC_QBUF
//107 enum v4l2_field {
//108     V4L2_FIELD_ANY           = 0, /* driver can choose from none,
//109                      top, bottom, interlaced
//110                      depending on whatever it thinks
//111                      is approximate ... */
//112     V4L2_FIELD_NONE          = 1, /* this device has no fields ... */
//113     V4L2_FIELD_TOP           = 2, /* top field only */
//114     V4L2_FIELD_BOTTOM        = 3, /* bottom field only */
//115     V4L2_FIELD_INTERLACED    = 4, /* both fields interlaced */
//116     V4L2_FIELD_SEQ_TB        = 5, /* both fields sequential into one
//117                      buffer, top-bottom order */
//118     V4L2_FIELD_SEQ_BT        = 6, /* same as above + bottom-top order */
//119     V4L2_FIELD_ALTERNATE     = 7, /* both fields alternating into
//120                      separate buffers */
//121     V4L2_FIELD_INTERLACED_TB = 8, /* both fields interlaced, top field
//122                      first and the top field is
//123                      transmitted first */
//124     V4L2_FIELD_INTERLACED_BT = 9, /* both fields interlaced, top field
//125                      first and the bottom field is
//126                      transmitted first */
//127 };
//--------------------------------------------------------------------------
//20 struct timeval {
//21     __kernel_time_t     tv_sec;     /* seconds */
//22     __kernel_suseconds_t    tv_usec;    /* microseconds */
//23 };
//--------------------------------------------------------------------------
//517 struct v4l2_timecode {
//518     __u32   type;
//519     __u32   flags;
//520     __u8    frames;
//521     __u8    seconds;
//522     __u8    minutes;
//523     __u8    hours;
//524     __u8    userbits[4];
//525 };
//--------------------------------------------------------------------------
//599 struct v4l2_plane {
//600     __u32           bytesused;
//601     __u32           length;
//602     union {
//603         __u32       mem_offset;
//604         unsigned long   userptr;
//605     } m;
//606     __u32           data_offset;
//607     __u32           reserved[11];
//608 };
//--------------------------------------------------------------------------
//637 struct v4l2_buffer {
//638     __u32                 index;
//639     enum v4l2_buf_type    type;
//640     __u32                 bytesused;
//641     __u32                 flags;
//642     enum v4l2_field       field;
//643     struct timeval        timestamp;
//644     struct v4l2_timecode  timecode;
//645     __u32                 sequence;
//646 
//647     /* memory location */
//648     enum v4l2_memory      memory;
//649     union {
//650         __u32             offset;
//651         unsigned long     userptr;
//652         struct v4l2_plane *planes;
//653     } m;
//654     __u32                 length;
//655     __u32                 input;
//656     __u32                 reserved;
//657 };
//

int camera_query_buffer(CameraDevice *cam)
{
    struct v4l2_buffer buf;  
    int i,ret=-1;  
    for (i = 0; i < cam->buffer_num; i++) {  
        memset(&buf, 0, sizeof buf);  
        buf.index = i;  
        buf.type = cam->type;  
        buf.memory = cam->memory_type;  
        ret = ioctl(cam->fd, VIDIOC_QUERYBUF, &buf);  
        if (ret < 0) {  
            CAM_DBG("Unable to query buffer %u (%d).\n", i, errno);  
            close(cam->fd);  
            return 1;  
        }  
        //CAM_DBG("length: %u offset: %x\n", buf.length, buf.m.offset);  
        CAM_DBG("\n");
        CAM_DBG("bytesused = %u, flags = %u, field = %u;\n"
//        "\tframes = %u, seconds = %u, minutes = %u, hours = %u;\n"
        "\tsequence = %u, length = %d, input = %u;\n"
        "\toffset = 0x%x, userptr = 0x%lx;\n",
        buf.bytesused, buf.flags, buf.field,
//        buf.timecode.frames, buf.timecode.seconds, buf.timecode.minutes, buf.timecode.hours,
        buf.sequence, buf.length, buf.input,
        buf.m.offset, buf.m.userptr);
#ifdef USE_PMAP
        cam->cam_buffer[i] = buf.m.offset;
#endif
#ifndef USE_PMAP
        cam->cam_buffer[i] = mmap(0, buf.length, PROT_READ, MAP_SHARED, cam->fd, buf.m.offset);  
        if (cam->cam_buffer[i] == MAP_FAILED) {  
            CAM_DBG("Unable to map buffer %u (%d)\n", i, errno);  
            close(cam->fd);  
            return 1;  
        }  
#endif
        CAM_DBG("Buffer %u mapped at address %p.\n", buf.index, cam->cam_buffer[i]);  
    }
    return 0;
}    

int camera_queue_buffer(CameraDevice *cam)
{
    struct v4l2_buffer buf;  
    int i,ret=-1;          
    /* Queue the buffers. */  
    //for (i = 0; i < cam->buffer_num; ++i) {  
        memset(&buf, 0, sizeof buf);  
        //buf.index = i;  
        buf.type = cam->type;  
        buf.memory = cam->memory_type;  
        ret = ioctl(cam->fd, VIDIOC_QBUF, &buf);  
        if (ret < 0) {  
            CAM_DBG("Unable to queue buffer (%d).\n", errno);  
            close(cam->fd);  
            return 1;  
        }
        else
           ;// CAM_DBG("QBUF ok!!!\n");
    //}     

    return 0;
}

/////////////////////////////////////////////////////////////////////////////////VIDIOC_STREAMON


int camera_streamon(CameraDevice *cam)  
{  
    int ret;
    //int buf_type=type;  
      
    ret = ioctl(cam->fd, VIDIOC_STREAMON, &cam->type);  
    if (ret < 0) {  
        CAM_DBG("Unable to start camera: %d.\n", errno);  
        return ret;  
    }
    CAM_DBG("Start Streaming...\n");

    return 0;  
}  
/////////////////////////////////////////////////////////////////////////////////VIDIOC_STREAMOFF
int camera_streamoff(CameraDevice *cam)
{
    int ret;
    //int buf_type=type;  
      
    ret = ioctl(cam->fd, VIDIOC_STREAMOFF, &cam->type);  
    if (ret < 0) {  
        CAM_DBG("Unable to stop camera: %d.\n", errno);  
        return ret;  
    }
    CAM_DBG("Stop Streaming...\n"); 
    return 0;  
    
}

/////////////////////////////////////////////////////////////////////////////////VIDIOC_DQBUF
int camera_dequeue_buffer(CameraDevice *cam)
{
    struct v4l2_buffer buf;  
    int i,ret=-1;          
    /* Queue the buffers. */  
    //for (i = 0; i < cam->buffer_num; ++i) {  
        memset(&buf, 0, sizeof buf);  
        //buf.index = i;  
        buf.type = cam->type;  
        buf.memory = cam->memory_type;  
        ret = ioctl(cam->fd, VIDIOC_DQBUF, &buf);  
        if (ret < 0) {  
            CAM_DBG("Unable to dequeue buffer (%d).\n", errno);  
            close(cam->fd);  
            return 1;  
        }
        else
            ;//CAM_DBG("DQBUF ok!!!\n");
    //}    
    return 0;
}

int camera_release_buffer(CameraDevice *cam)
{
	struct v4l2_buffer buf;
	int i;

	memset(&buf, 0,  sizeof(struct v4l2_buffer));

	buf.type = cam->type;
	buf.memory = cam->memory_type;
	buf.index = 0;
	for (i=0; i<cam->buffer_num; i++) 
	{
	    if ( ioctl(cam->fd, VIDIOC_QUERYBUF, &buf) < 0) 
		{
			break;
	    }
	    CAM_DBG("buf.index = %d, buf.length = %d...\n",buf.index, buf.length);
	    //munmap(cam->cam_buffer[buf.index], buf.length);
	}
	return 0;
}

