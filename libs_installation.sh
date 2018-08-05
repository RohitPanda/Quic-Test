#!/bin/bash
mkdir -p tests
TEST_DIR=$PWD/tests

sudo rm -rf ./build
mkdir build
cd build

PKG_OK=$(cmake --version|grep "cmake version 3.10")
echo Checking for CMake: $PKG_OK
if [ "" == "$PKG_OK" ]; then
  echo "No CMake. Setting up."
  wget https://cmake.org/files/v3.10/cmake-3.10.3-Linux-x86_64.tar.gz|| exit -1
  tar -xf cmake-3.10.3-Linux-x86_64.tar.gz
  export PATH=$PWD/cmake-3.10.3-Linux-x86_64/bin:$PATH
fi  

PKG_OK=$(go version|grep "go version go")
echo Checking for Go: $PKG_OK
if [ "" == "$PKG_OK" ]; then
  echo "No GO. Setting up."
  wget https://dl.google.com/go/go1.10.3.linux-amd64.tar.gz|| exit -1
  tar -xzf go1.10.3.linux-amd64.tar.gz 
  export PATH=$PWD/go/bin:$PATH
fi

PKG_OK=$(ldconfig -p | grep libz.so$)
echo Checking for Zlib: $PKG_OK
if [ "" == "$PKG_OK" ]; then
  echo "No Zlib. Setting up."
  wget https://zlib.net/zlib-1.2.11.tar.gz|| exit -1
  tar -xf zlib-1.2.11.tar.gz
  cd zlib-1.2.11/
  make distclean
  ./configure
  make
  sudo make install
  cd ..
fi

git clone https://boringssl.googlesource.com/boringssl
cd boringssl
git checkout chromium-stable
cmake . &&  make
  cmake -DCMAKE_BUILD_TYPE=Release -DBUILD_SHARED_LIBS=1 . && make
BORINGSSL_SOURCE=$PWD
mkdir -p lib
cd lib
  ln -s $BORINGSSL_SOURCE/ssl/libssl.a
  ln -s $BORINGSSL_SOURCE/crypto/libcrypto.a
cd ..
cd ..

PKG_OK=$(yasm --version|grep "yasm 1.")
echo Checking for yasm: $PKG_OK
if [ "" == "$PKG_OK" ]; then
  echo "No yasm. Setting up."
  wget https://www.tortall.net/projects/yasm/releases/yasm-1.2.0.tar.gz|| exit -1
  tar -xf yasm-1.2.0.tar.gz
  cd yasm-1.2.0
  ./configure
  make
  sudo make install
  cd ..
fi

CURL_VERSION=7.58.0

wget "https://curl.haxx.se/download/curl-${CURL_VERSION}.tar.gz"|| exit -1
tar -xf curl-${CURL_VERSION}.tar.gz
cd curl-${CURL_VERSION}
mkdir -p build
  LIBS="-ldl -lm -lpthread -lz" ./configure --with-ssl=${BORINGSSL_SOURCE} --prefix=$PWD/build --disable-shared --enable-ipv6 --disable-manual --disable-dict --disable-file --disable-file --disable-ftp --disable-gopher --disable-imap --disable-pop3 --disable-rtsp --disable-smtp --disable-telnet --disable-tftp --disable-smb
  make
  make install
cd ..

FFMPEG_VERSION=4.0

wget "https://ffmpeg.org/releases/ffmpeg-${FFMPEG_VERSION}.tar.gz"|| exit -1
tar -xf ffmpeg-${FFMPEG_VERSION}.tar.gz
cd ffmpeg-${FFMPEG_VERSION}
mkdir -p build
./configure --enable-shared --enable-static --prefix=$PWD/build --disable-all --enable-avformat --enable-avcodec --enable-avutil --enable-demuxer=mov --enable-demuxer=matroska --enable-demuxer=flv --disable-debug --enable-lto --enable-small --disable-zlib --disable-bzlib --disable-pthreads --enable-protocol=file
make
make install
cd ..

PKG_OK=$(ldconfig -p | grep /lib/libevent)
echo Checking for libevent: $PKG_OK
if [ "" == "$PKG_OK" ]; then
  echo "No libevent. Setting up."
  wget https://github.com/libevent/libevent/releases/download/release-2.1.8-stable/libevent-2.1.8-stable.tar.gz || exit -1
  tar -xf libevent-2.1.8-stable.tar.gz

  cd libevent-2.1.8-stable
  ./configure --enable-shared=0
  make
  sudo make install
  cd ..
fi

git clone https://github.com/litespeedtech/lsquic-client.git
cd lsquic-client
cmake -DBORINGSSL_INCLUDE=$BORINGSSL_SOURCE/include \
                                -DBORINGSSL_LIB=$BORINGSSL_SOURCE/lib .
make
cd ..

wget "https://www.caida.org/tools/measurement/scamper/code/scamper-cvs-20180504.tar.gz"|| exit -1
tar -xzf scamper-cvs-20180504.tar.gz
cd scamper-cvs-20180504
mkdir -p build
./configure
make
sudo make install
cd ..

cd ..

cd youtube_tcp_test
mkdir -p build
cd build
cmake ..
make
cp youtube_tcp_test $TEST_DIR/.
cd ..
cd ..

cd quic_probe
mkdir -p build
cd build
cmake ..
make
cp libquic_client.a ../.
cd ..
cd ..

cd youtube_test
mkdir -p build
cd build
cmake ..
make
cp youtube_test $TEST_DIR/.
cd ..
cd ..
