#!/bin/bash


set -e

VERSION="7.27.0"
LIBNAME="libcurl"
LIBDOWNLOAD="http://curl.haxx.se/download/curl-${VERSION}.tar.gz"
ARCHIVE="${LIBNAME}-${VERSION}.tar.gz"

# Enabled/disabled protocols (the fewer, the smaller the final binary size)
PROTOCOLS="--enable-http --disable-rtsp --disable-ftp --disable-file --disable-ldap --disable-ldaps \
           --disable-rtsp --disable-dict --disable-telnet --disable-tftp \
           --disable-pop3 --disable-imap --disable-smtp --disable-gopher"

CONFIGURE_FLAGS="--with-darwinssl --without-ssl --without-libssh2 --without-librtmp --without-ca-bundle ${PROTOCOLS}"

DIR=`pwd`
MACOSX_MIN="10.6"
ARCHS="i386 x86_64"


# Download or use existing tar.gz
if [ ! -e ${ARCHIVE} ]
then
    echo ""
    echo "* Downloading ${ARCHIVE}"
    echo ""
    curl -o ${ARCHIVE} ${LIBDOWNLOAD}
else
    echo ""
    echo "* Using ${ARCHIVE}"
fi


# Create out dirs
mkdir -p "${DIR}/bin"
mkdir -p "${DIR}/lib"
mkdir -p "${DIR}/src"


# Build for all archs
for ARCH in ${ARCHS}
do

    echo ""
    echo "* Building ${LIBNAME} ${VERSION}  (${ARCH})..."

    # Expand source code, prepare output directory and set log
    tar zxf ${ARCHIVE} -C "${DIR}/src"
    rm -rf "${DIR}/src/${LIBNAME}-${VERSION}"
    mv -f "${DIR}/src/curl-${VERSION}" "${DIR}/src/${LIBNAME}-${VERSION}"

    mkdir -p "${DIR}/bin/${LIBNAME}-${VERSION}/${ARCH}"

    cd "${DIR}/src/${LIBNAME}-${VERSION}"

    # compilation binaries
    export CC="$(xcrun -find clang)"
    export LD="$(xcrun -find ld)"
    export AR="$(xcrun -find ar)"
    export AS="$(xcrun -find as)"
    export NM="$(xcrun -find nm)"
    export RANLIB="$(xcrun -find ranlib)"

    # compilation flags
    export LDFLAGS="-arch ${ARCH} -pipe -L${DIR}/lib"
    export CFLAGS="-arch ${ARCH} -pipe -I${DIR}/include -mmacosx-version-min=${MACOSX_MIN}"

    ./configure --host=${ARCH}-apple-darwin --disable-shared --enable-static ${CONFIGURE_FLAGS} \
                --prefix="${DIR}/bin/${LIBNAME}-${VERSION}/${ARCH}"

    cp include/curl/curlbuild.h ${DIR}/curlbuild-${ARCH}.h

    make
    make install

    
    cd ${DIR}
    rm -rf "${DIR}/src/${LIBNAME}-${VERSION}"
done


echo ""
echo "* Creating binaries for ${LIBNAME}..."

I386_LIB="${DIR}/bin/${LIBNAME}-${VERSION}/i386/lib/${LIBNAME}.a"
X86_64_LIB="${DIR}/bin/${LIBNAME}-${VERSION}/x86_64/lib/${LIBNAME}.a"

# Create a single .a file for all architectures
if [ -e ${I386_LIB} -a -e ${X86_64_LIB} ]
then
    lipo -create  ${X86_64_LIB} ${I386_LIB} -output "${DIR}/lib/${LIBNAME}.a"
fi

# Create a single .a file for i386 (iphonesimulator or macosx both generate the exact same output)
if [ -e ${I386_LIB} ]
then
    lipo -create ${I386_LIB} -output "${DIR}/lib/${LIBNAME}-i386.a"
fi

# Create a single .a file for x86_64, just for fun
if [ -e ${X86_64_LIB} ]
then
    lipo -create ${X86_64_LIB} -output "${DIR}/lib/${LIBNAME}-x86_64.a"
fi


# Copy the header files to include
mkdir -p "${DIR}/include/"
FIRST_ARCH="${ARCHS%% *}"
ditto "${DIR}/bin/${LIBNAME}-${VERSION}/${FIRST_ARCH}/include/" \
      "${DIR}/include/"
ditto "${DIR}"/curlbuild-*.h "${DIR}/include/curl"

cat > "${DIR}/include/curl/curlbuild.h" <<EOF
#if defined(__LP64__)
# include "curlbuild-x86_64.h"
#else
# include "curlbuild-i386.h"
#endif 
EOF

echo ""
echo "* Finished; ${LIBNAME} binary created for platforms: ${ARCHS}"
