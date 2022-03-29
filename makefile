BUILD = bin
EMSCRIPTEN_BUILD = embin
SERVER_DIR = ./backend/
WASM_OUT   = ./hosted/generated/
USR_DIR    =/usr
INCS_DIRS  =-I${USR_DIR}/include -Iinclude -I./include
LIBS_DIRS  =-L${USR_DIR}/lib
CPP_DEFS   =-D=HAVE_CONFIG_H
CPP_OPTS   = -O3 -Wno-pessimizing-move
#-Wall
LIBS       = -levent -levent_core


SRC		   = ${SERVER_DIR}source/WebServer.cpp \
			${SERVER_DIR}source/TableServer.cpp \
			${SERVER_DIR}source/SyncServer.cpp
SRC_INC	   = -I${SERVER_DIR}include

API_DIR    =./wasm/
API_INC	   = -I${API_DIR}include -I${API_DIR}source
EXPORTED_FUNCTIONS =[\
	'_setPacketPointer', \
	'_setModel', \
	'_getUpdatedBuffers', \
	'_rayTrace', \
	'_scan', \
	'_testAllocate', \
	'_malloc', \
	'_free']
EXTRA_EXPORTED_RUNTIME_METHODS=['ccall']
API_MAIN = ${API_DIR}source/api.cpp
API_SRC    =${API_DIR}source/Variant.cpp \
        	${API_DIR}source/GLTF.cpp

default: all

all: wasm
#server: ${SERVER_DIR}source/Main.cpp
#	g++ -pthread -std=c++17 ${CPP_OPTS} ${CPP_DEFS} -o Main.exe -I${API_DIR} ${API_INC} ${SRC_INC} ${INCS_DIRS} ${SERVER_DIR}source/Main.cpp ${API_SRC} ${SRC} ${LIBS_DIRS} ${LIBS} 
#mv ${BUILD}/bin/server backend/Main.exe
wasm: ${API_DIR}source/api.cpp
	emcc -std=c++17 -g -s EXPORT_NAME="initializeCPPAPI" -s MODULARIZE=1 -s ALLOW_MEMORY_GROWTH=1 -s MAXIMUM_MEMORY=4GB -O3 -s ASSERTIONS=1 ${API_MAIN} ${API_SRC} --post-js ${API_DIR}source/api_post.js -o ${WASM_OUT}api.js ${API_INC} -s "EXPORTED_FUNCTIONS=${EXPORTED_FUNCTIONS}" -s "EXPORTED_RUNTIME_METHODS=${EXTRA_EXPORTED_RUNTIME_METHODS}"
clean:
	$(RM) -r ${SERVER_OUT}Main.exe ${WASM_OUT}api.js ${WASM_OUT}api.wasm
