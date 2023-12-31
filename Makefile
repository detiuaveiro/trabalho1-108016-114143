# make              # to compile files and create the executables
# make pgm          # to download example images to the pgm/ dir
# make setup        # to setup the test files in test/ dir
# make tests        # to run basic tests
# make clean        # to cleanup object files and executables
# make cleanobj     # to cleanup object files only

CFLAGS = -Wall -O2 -g

PROGS = imageTool imageTest

TESTS = test1 test2 test3 test4 test5 test6 test7 test8 test9

# Default rule: make all programs
all: $(PROGS)

imageTest: imageTest.o image8bit.o instrumentation.o error.o

imageTest.o: image8bit.h instrumentation.h

imageTool: imageTool.o image8bit.o instrumentation.o error.o

imageTool.o: image8bit.h instrumentation.h

# Rule to make any .o file dependent upon corresponding .h file
%.o: %.h

pgm:
	wget -O- https://sweet.ua.pt/jmr/aed/pgm.tgz | tar xzf -

.PHONY: setup
setup: test/

test/:
	wget -O- https://sweet.ua.pt/jmr/aed/test.tgz | tar xzf -
	@#mkdir -p $@
	@#curl -s -o test/aed-trab1-test.zip https://sweet.ua.pt/mario.antunes/aed/test/aed-trab1-test.zip
	@#unzip -q -o test/aed-trab1-test.zip -d test/

test1: $(PROGS) setup
	./imageTool test/original.pgm neg save neg.pgm
	cmp neg.pgm test/neg.pgm

test2: $(PROGS) setup
	./imageTool test/original.pgm thr 128 save thr.pgm
	cmp thr.pgm test/thr.pgm

test3: $(PROGS) setup
	./imageTool test/original.pgm bri .33 save bri.pgm
	cmp bri.pgm test/bri.pgm

test4: $(PROGS) setup
	./imageTool test/original.pgm rotate save rotate.pgm
	cmp rotate.pgm test/rotate.pgm

test5: $(PROGS) setup
	./imageTool test/original.pgm mirror save mirror.pgm
	cmp mirror.pgm test/mirror.pgm

test6: $(PROGS) setup
	./imageTool test/original.pgm crop 100,100,100,100 save crop.pgm
	cmp crop.pgm test/crop.pgm

test7: $(PROGS) setup
	./imageTool test/small.pgm test/original.pgm paste 100,100 save paste.pgm
	cmp paste.pgm test/paste.pgm

test8: $(PROGS) setup
	./imageTool test/small.pgm test/original.pgm blend 100,100,.33 save blend.pgm
	cmp blend.pgm test/blend.pgm

test9: $(PROGS) setup
	./imageTool test/original.pgm blur 7,7 save blur.pgm
	cmp blur.pgm test/blur.pgm

test10: $(PROGS) setup
	./imageTool test/chess8.pgm blur 1,1 save blur2.pgm


test11: $(PROGS) setup
	./imageTool pgm/medium/ireland-03_640x480.pgm blur 8,8 save blur3.pgm

test12: $(PROGS) setup
	./imageTool pgm/large/ireland_03_1600x1200.pgm blur 8,8 save blur4.pgm

test13: $(PROGS) setup
	./imageTool pgm/small/art3_222x217.pgm blur 0,4 save blur5.pgm

test14: $(PROGS) setup
	./imageTool pgm/small/art3_222x217.pgm blur 4,0 save blur6.pgm

test15: $(PROGS) setup
	./imageTool pgm/small/bird_256x256.pgm blur 5,2 save blur7.pgm

test16: $(PROGS) setup
	./imageTool pgm/large/airfield-05_1600x1200.pgm blur 15,15 save blur8.pgm

test20: $(PROGS) setup
	./imageTool test/crop.pgm test/original.pgm locate

test21:
	./imageTool test/small.pgm test/paste.pgm locate

test22:
	./imageTool pgm/small/bird_256x256.pgm crop 0,0,1,1 save small2.pgm
	./imageTool small2.pgm pgm/small/bird_256x256.pgm locate

test23:
	./imageTool pgm/small/bird_256x256.pgm crop 30,180,7,5 save small2.pgm
	./imageTool small2.pgm pgm/small/bird_256x256.pgm locate

test24:
	./imageTool pgm/large/ireland-06-1200x1600.pgm crop 0,0,3,3 save small3.pgm
	./imageTool small3.pgm pgm/large/ireland-06-1200x1600.pgm locate

test25:
	./imageTool pgm/large/ireland-06-1200x1600.pgm crop 570,890,100,300 save small3.pgm
	./imageTool small3.pgm pgm/large/ireland-06-1200x1600.pgm locate

test26:
	./imageTool test/chess8.pgm crop 3,3,2,2 save small4.pgm
	./imageTool small4.pgm test/chess8.pgm locate
test27:
	./imageTool test/chess8.pgm test/original.pgm  locate
	
.PHONY: tests
tests: $(TESTS)

# Make uses builtin rule to create .o from .c files.

cleanobj:
	rm -f *.o

clean: cleanobj
	rm -f $(PROGS)

