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
XCODE_SELECT="xcode-select"
XCODE=$(${XCODE_SELECT} --print-path)
SDK_VERSION="6.0"
IOS_MIN="5.1"
ARCHS="i386 armv7"


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
    if [ "${ARCH}" == "armv7" ]
    then
        PLATFORM="iPhoneOS"
    else
        PLATFORM="iPhoneSimulator"
    fi

    echo ""
    echo "* Building ${LIBNAME} ${VERSION}  (${ARCH})..."

    # Expand source code, prepare output directory and set log
    tar zxf ${ARCHIVE} -C "${DIR}/src"
    rm -rf "${DIR}/src/${LIBNAME}-${VERSION}"
    mv -f "${DIR}/src/curl-${VERSION}" "${DIR}/src/${LIBNAME}-${VERSION}"

    mkdir -p "${DIR}/bin/${LIBNAME}-${VERSION}/${ARCH}"

    cd "${DIR}/src/${LIBNAME}-${VERSION}"

    # compilation binaries
    XCRUN_SDK=$(echo ${PLATFORM} | tr '[:upper:]' '[:lower:]')
    export CC="$(xcrun -sdk ${XCRUN_SDK} -find clang)"
    export LD="$(xcrun -sdk ${XCRUN_SDK} -find ld)"
    export AR="$(xcrun -sdk ${XCRUN_SDK} -find ar)"
    export AS="$(xcrun -sdk ${XCRUN_SDK} -find as)"
    export NM="$(xcrun -sdk ${XCRUN_SDK} -find nm)"
    export RANLIB="$(xcrun -sdk ${XCRUN_SDK} -find ranlib)"

    # compilation flags
    SDK="${XCODE}/Platforms/${PLATFORM}.platform/Developer/SDKs/${PLATFORM}${SDK_VERSION}.sdk"
    export LDFLAGS="-arch ${ARCH} -pipe -isysroot ${SDK} -L${DIR}/lib -miphoneos-version-min=${IOS_MIN}"
    export CFLAGS="-arch ${ARCH} -pipe -isysroot ${SDK} -I${DIR}/include -miphoneos-version-min=${IOS_MIN}"

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
ARMV7_LIB="${DIR}/bin/${LIBNAME}-${VERSION}/armv7/lib/${LIBNAME}.a"

# Create a single .a file for all architectures
if [ -e ${I386_LIB} -a -e ${ARMV7_LIB} ]
then
    lipo -create  ${I386_LIB} ${ARMV7_LIB} -output "${DIR}/lib/${LIBNAME}.a"
fi

# Create a single .a file for all arm architectures
if [ -e ${ARMV7_LIB} ]
then
    lipo -create ${ARMV7_LIB} -output "${DIR}/lib/${LIBNAME}-armv7.a"
fi

# Create a single .a file for i386 (iphonesimulator or macosx both generate the exact same output)
if [ -e ${I386_LIB} ]
then
    lipo -create ${I386_LIB} -output "${DIR}/lib/${LIBNAME}-i386.a"
fi


# Copy the header files to include
mkdir -p "${DIR}/include/"
FIRST_ARCH="${ARCHS%% *}"
ditto "${DIR}/bin/${LIBNAME}-${VERSION}/${FIRST_ARCH}/include/" \
      "${DIR}/include/"
ditto "${DIR}"/curlbuild-*.h "${DIR}/include/curl"

cat > "${DIR}/include/curl/curlbuild.h" <<EOF
#if defined (__arm__)
# include "curlbuild-armv7.h"
#else
# include "curlbuild-i386.h"
#endif 
EOF

echo ""
echo "* Finished; ${LIBNAME} binary created for platforms: ${ARCHS}"
