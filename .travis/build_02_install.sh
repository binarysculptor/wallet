#!/usr/bin/env bash
#
# Copyright (c) 2018 The Bitcoin Core developers
# Distributed under the MIT software license, see the accompanying
# file COPYING or http://www.opensource.org/licenses/mit-license.php.

export LC_ALL=C.UTF-8

travis_retry docker pull "$DOCKER_NAME_TAG"
env | grep -E '^(CCACHE_|WINEDEBUG|LC_ALL|BOOST_TEST_RANDOM|CONFIG_SHELL)' | tee /tmp/env
if [[ $HOST = *-mingw32 ]]; then
  DOCKER_ADMIN="--cap-add SYS_ADMIN"
fi
DOCKER_ID=$(docker run $DOCKER_ADMIN -idt --mount type=bind,src=$TRAVIS_BUILD_DIR,dst=$TRAVIS_BUILD_DIR --mount type=bind,src=$CCACHE_DIR,dst=$CCACHE_DIR -w $TRAVIS_BUILD_DIR --env-file /tmp/env $DOCKER_NAME_TAG)

DOCKER_EXEC () {
  docker exec $DOCKER_ID bash -c "cd $PWD && $*"
}

if [ -n "$DPKG_ADD_ARCH" ]; then
  DOCKER_EXEC dpkg --add-architecture "$DPKG_ADD_ARCH"
fi

mkdir -p /home/travis/build/project-liberty/wallet/build/
#DOCKER_EXEC mkdir -p depends/SDKs depends/sdk-sources

#DOCKER_EXEC usermod -aG travis
cat /etc/passwd
travis_retry DOCKER_EXEC apt-get update 
travis_retry DOCKER_EXEC apt-get -y install sudo
travis_retry DOCKER_EXEC apt-get -y install libdb4.8-dev libdb4.8++-dev
travis_retry DOCKER_EXEC apt-get install --no-install-recommends --no-upgrade -qq $PACKAGES $DOCKER_PACKAGES
travis_retry DOCKER_EXEC chown -R travis:travis /home/travis