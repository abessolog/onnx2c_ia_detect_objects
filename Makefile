GPU=0
CUDNN=0
OPENCV=1
OPENMP=0
DEBUG=1

ARCH= -gencode arch=compute_30,code=sm_30 \
      -gencode arch=compute_35,code=sm_35 \
      -gencode arch=compute_50,code=[sm_50,compute_50] \
      -gencode arch=compute_52,code=[sm_52,compute_52]
#      -gencode arch=compute_20,code=[sm_20,sm_21] \ This one is deprecated?

# This is what I use, uncomment if you know your arch and want to specify
# ARCH= -gencode arch=compute_52,code=compute_52

VPATH=./src/:./include/:./GEN-YOLOV2/:./externals/stb/
SLIB=libstb.so
ALIB=libstb.a
ATYLIB=libtinyyolov2-7.a
STYLIB=libtinyyolov2-7.so
EXEC=ia_detect
OBJDIR=./obj/

CC=gcc
CPP=g++
NVCC=nvcc 
AR=ar
ARFLAGS=rcs
OPTS=-Ofast
LDFLAGS= -lm -lpthread 
COMMON=-Iinclude/
CFLAGS=-Wall -Wno-unused-result -Wno-unknown-pragmas -Wfatal-errors -fPIC

ifeq ($(OPENMP), 1) 
CFLAGS+= -fopenmp
endif

ifeq ($(DEBUG), 1) 
OPTS=-O0 -g
endif

CFLAGS+=$(OPTS)

ifeq ($(OPENCV), 1) 
COMMON+= -DOPENCV
CFLAGS+= -DOPENCV
LDFLAGS+= `pkg-config --libs opencv4` -lstdc++
COMMON+= `pkg-config --cflags opencv4` 
endif

ifeq ($(GPU), 1) 
COMMON+= -DGPU -I/usr/local/cuda/include/
CFLAGS+= -DGPU
LDFLAGS+= -L/usr/local/cuda/lib64 -lcuda -lcudart -lcublas -lcurand
endif

ifeq ($(CUDNN), 1) 
COMMON+= -DCUDNN 
CFLAGS+= -DCUDNN
LDFLAGS+= -lcudnn
endif


EXECOBJA=ia_detect.o parser.o 
OBJTY=tensor_BatchNormalization_B.o \
tensor_BatchNormalization_B1.o \
tensor_BatchNormalization_B2.o \
tensor_BatchNormalization_B3.o \
tensor_BatchNormalization_B4.o \
tensor_BatchNormalization_B5.o \
tensor_BatchNormalization_B6.o \
tensor_BatchNormalization_B7.o \
tensor_BatchNormalization_mean.o \
tensor_BatchNormalization_mean1.o \
tensor_BatchNormalization_mean2.o \
tensor_BatchNormalization_mean3.o \
tensor_BatchNormalization_mean4.o \
tensor_BatchNormalization_mean5.o \
tensor_BatchNormalization_mean6.o \
tensor_BatchNormalization_mean7.o \
tensor_BatchNormalization_scale.o \
tensor_BatchNormalization_scale1.o \
tensor_BatchNormalization_scale2.o \
tensor_BatchNormalization_scale3.o \
tensor_BatchNormalization_scale4.o \
tensor_BatchNormalization_scale5.o \
tensor_BatchNormalization_scale6.o \
tensor_BatchNormalization_scale7.o \
tensor_BatchNormalization_variance.o \
tensor_BatchNormalization_variance1.o \
tensor_BatchNormalization_variance2.o \
tensor_BatchNormalization_variance3.o \
tensor_BatchNormalization_variance4.o \
tensor_BatchNormalization_variance5.o \
tensor_BatchNormalization_variance6.o \
tensor_BatchNormalization_variance7.o \
tensor_convolution1_W.o \
tensor_convolution2_W.o \
tensor_convolution3_W.o \
tensor_convolution4_W.o \
tensor_convolution5_W.o \
tensor_convolution6_W.o \
tensor_convolution7_W.o \
tensor_convolution8_W.o \
tensor_convolution8_B.o \
tensor_convolution_W.o \
tensor_scalerPreprocessor_bias.o \
tensor_scalerPreprocessor_scale.o \
entry.o \
node_activation.o \
node_activation1.o \
node_activation2.o \
node_activation3.o \
node_activation4.o \
node_activation5.o \
node_activation6.o \
node_activation7.o \
node_batchnorm1.o \
node_batchnorm3.o \
node_batchnorm5.o \
node_convolution.o \
node_convolution2.o \
node_convolution4.o \
node_convolution6.o \
node_convolution7.o \
node_convolution8.o \
node_Mul.o \
node_pooling.o \
node_pooling2.o \
node_pooling4.o 

OBJ=stb_vorbis.o   \
stb_c_lexer.o \
stb_connected_components.o \
stb_divide.o \
stb_ds.o \
stb_dxt.o \
stb_easy_font.o \
stb_herringbone_wang_tile.o \
stb_hexwave.o \
stb_image.o \
stb_image_resize2.o \
stb_image_write.o \
stb_include.o \
stb_leakcheck.o \
stb_perlin.o \
stb_rect_pack.o \
stb_sprintf.o \
stb_textedit.o \
stb_tilemap_editor.o \
stb_truetype.o \
stb_voxel_render.o 

ifeq ($(GPU), 1) 
LDFLAGS+= -lstdc++ 
OBJ+=
endif

EXECOBJ = $(addprefix $(OBJDIR), $(EXECOBJA))
OBJS = $(addprefix $(OBJDIR), $(OBJ))
DEPS = $(wildcard include/*.h)
OBJTYS = $(addprefix $(OBJDIR), $(OBJTY))


all: obj backup results  $(ALIB) $(ATYLIB) $(STYLIB) $(EXEC) 



$(EXEC): $(EXECOBJ) $(ALIB) $(ATYLIB)
	$(CC) $(COMMON) $(CFLAGS) $^ -o $@ $(LDFLAGS) $(ALIB)

$(ALIB): $(OBJS)
	$(AR) $(ARFLAGS) $@ $^

$(ATYLIB): $(OBJTYS)
	$(AR) $(ARFLAGS) $@ $^

$(SLIB): $(OBJS)
	$(CC) $(CFLAGS)  $^  -shared -o $@ $(LDFLAGS)

$(STYLIB): $(OBJTYS)
	$(CC) $(CFLAGS)  $^  -shared -o $@ $(LDFLAGS)

$(OBJDIR)%.o: %.cpp 
	$(CPP) $(COMMON) $(CFLAGS) -c $< -o $@

$(OBJDIR)%.o: %.c 
	$(CC) $(COMMON) $(CFLAGS) -c $< -o $@

$(OBJDIR)%.o:  %.h
	$(CC) $(COMMON) $(CFLAGS) -c $< -o $@

$(OBJDIR)%.o: %.cu 
	$(NVCC) $(ARCH) $(COMMON) --compiler-options "$(CFLAGS)" -c $< -o $@

obj:
	mkdir -p obj
backup:
	mkdir -p backup
results:
	mkdir -p results

.PHONY: clean

clean:
	rm -rf $(OBJS) $(SLIB) $(ALIB) $(ATYLIB) $(STYLIB) $(EXEC) $(EXECOBJ) $(OBJDIR) $(OBJTYS)

