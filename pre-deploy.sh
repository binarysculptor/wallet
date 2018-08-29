#!/bin/bash

CONFIG_FILE=$TRAVIS_BUILD_DIR/src/config/liberty-config.h
VERSION_MAJOR=`sed -n 's/#define CLIENT_VERSION_MAJOR \([0-9]*$\)/\1/p' $CONFIG_FILE`
VERSION_MINOR=`sed -n 's/#define CLIENT_VERSION_MINOR \([0-9]*$\)/\1/p' $CONFIG_FILE`
VERSION_REVISION=`sed -n 's/#define CLIENT_VERSION_REVISION \([0-9]*$\)/\1/p' $CONFIG_FILE`
VERSION_BUILD=`sed -n 's/#define CLIENT_VERSION_BUILD \([0-9]*$\)/\1/p' $CONFIG_FILE`
CLIENT_VERSION="$VERSION_MAJOR.$VERSION_MINOR.$VERSION_REVISION.$VERSION_BUILD"

OUTDIR=$TRAVIS_BUILD_DIR/out/$TRAVIS_PULL_REQUEST/$TRAVIS_JOB_NUMBER-$HOST
mkdir -p $OUTDIR/bin

ARCHIVE_CMD="zip"
ARCHIVE_NAME=""

if [[ $HOST = "i686-w64-mingw32" ]]; then
  ARCHIVE_NAME="windows-x86.zip"
elif [[ $HOST = "x86_64-w64-mingw32" ]]; then
    ARCHIVE_NAME="windows-x64.zip"
elif [[ $HOST = "x86_64-unknown-linux-gnu" ]]; then
    if [[ $DEP_OPTS = "NO_WALLET=1" ]]; then    
        ARCHIVE_NAME="linux-x64-no-wallet.tar.gz"
        ARCHIVE_CMD="tar -czf"
    else
        ARCHIVE_NAME="linux-x64.tar.gz"
        ARCHIVE_CMD="tar -czf"
    fi  
elif [[ $HOST = "i686-pc-linux-gnu" ]]; then
    ARCHIVE_NAME="linux-x86.tar.gz"
    ARCHIVE_CMD="tar -czf"
elif [[ $HOST = "x86_64-apple-darwin11" ]]; then
    ARCHIVE_NAME="osx-x64.zip"
fi

if [[ $HOST = "x86_64-apple-darwin11" ]]; then
    find $TRAVIS_BUILD_DIR -type f | grep -i Liberty-Coin.dmg$ | xargs -i cp {} $OUTDIR/bin
else
    cp $TRAVIS_BUILD_DIR/src/qt/liberty-qt $OUTDIR/bin/ || cp $TRAVIS_BUILD_DIR/src/qt/liberty-qt.exe $OUTDIR/bin/ || echo "no QT Wallet"
    cp $TRAVIS_BUILD_DIR/src/libertyd $OUTDIR/bin/ || cp $TRAVIS_BUILD_DIR/src/liberty.exe $OUTDIR/bin/
    cp $TRAVIS_BUILD_DIR/src/liberty-cli $OUTDIR/bin/ || cp $TRAVIS_BUILD_DIR/src/liberty-cli.exe $OUTDIR/bin/
    strip "$OUTDIR/bin"/* || echo "nothing to strip"
fi

cd $OUTDIR/bin
ARCHIVE_CMD="$ARCHIVE_CMD $ARCHIVE_NAME *"
eval $ARCHIVE_CMD

mkdir -p $OUTDIR/zip
mv $ARCHIVE_NAME $OUTDIR/zip

sleep $[ ( $RANDOM % 6 )  + 1 ]s