#
# Makefile para Linux e macOS
#

SOURCE  = main.c lib/SOIL/image_DXT.c lib/SOIL/image_helper.c lib/SOIL/SOIL.c lib/SOIL/stb_image_aug.c
OBJECTS = $(SOURCE:.c=.o)
CFLAGS = -DGL_SILENCE_DEPRECATION

UNAME = `uname`

all: $(TARGET)
	-@make -j $(UNAME)

Darwin: $(OBJECTS)
	gcc $(OBJECTS) -O3 -Wno-deprecated -framework OpenGL -framework GLUT -framework Cocoa -lm -o codificador

Linux: $(OBJECTS)
	gcc $(OBJECTS) -O3 -lGL -lGLU -lglut -lm -o codificador

clean:
	-@ rm -f $(OBJECTS) codificador
