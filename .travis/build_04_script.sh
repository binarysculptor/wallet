#!/usr/bin/env bash
#
# Copyright (c) 2018 The Bitcoin Core developers
# Distributed under the MIT software license, see the accompanying
# file COPYING or http://www.opensource.org/licenses/mit-license.php.

export LC_ALL=C.UTF-8

TRAVIS_COMMIT_LOG=$(git log --format=fuller -1)
export TRAVIS_COMMIT_LOG

OUTDIR=$BASE_OUTDIR/$TRAVIS_PULL_REQUEST/$TRAVIS_JOB_NUMBER-$HOST
BITCOIN_CONFIG_ALL="--disable-dependency-tracking --prefix=$TRAVIS_BUILD_DIR/depends/$HOST --bindir=$OUTDIR/bin --libdir=$OUTDIR/lib"
if [ -z "$NO_DEPENDS" ]; then
  DOCKER_EXEC ccache --max-size=$CCACHE_SIZE
fi

BEGIN_FOLD autogen
  if [ -n "$CONFIG_SHELL" ]; then
    DOCKER_EXEC $CONFIG_SHELL -c ./autogen.sh
  else
    DOCKER_EXEC "su travis -c ./autogen.sh"
  fi
END_FOLD

cd build || (echo "could not enter build directory"; exit 1)

BEGIN_FOLD configure
   DOCKER_EXEC 'su travis -c "../configure --cache-file=config.cache $BITCOIN_CONFIG_ALL $BITCOIN_CONFIG || ( cat config.log && false)"'
END_FOLD

#cd ..
pwd
ls -la
find /home/travis/build/project-liberty/wallet  -maxdepth 7 -name "Makefile"
#find /home/travis/build/project-liberty/wallet -name "makefile" -maxdepth 3

BEGIN_FOLD distdir
   DOCKER_EXEC "su travis -c 'make -C /home/travis/build/project-liberty/wallet/depends VERSION=$HOST'"
END_FOLD

cd "liberty-$HOST" || (echo "could not enter distdir liberty-$HOST"; exit 1)
pwd
BEGIN_FOLD configure
   DOCKER_EXEC 'su travis -c "./configure --cache-file=../config.cache $BITCOIN_CONFIG_ALL $BITCOIN_CONFIG || ( cat config.log && false)"'
END_FOLD
pwd
ls -la

DOCKER_EXEC "find /home/travis/build/project-liberty/wallet  -name 'Makefile'"
DOCKER_EXEC "find /home/travis/build/project-liberty/wallet  -name 'makefile'"
DOCKER_EXEC "find /home/travis/build/project-liberty/wallet  -name 'make'"
#DOCKER_EXEC "find /home/travis/build/project-liberty/wallet -maxdepth 3 -name 'Makefile'"
#DOCKER_EXEC "find /home/travis/build/project-liberty/wallet -maxdepth 3 -name 'makefile'"
#DOCKER_EXEC "find /home/travis/build  -maxdepth 5 -name 'config.cache'" >> found /home/travis/build/project-liberty/config.cache
echo "ls -la /home/travis"
DOCKER_EXEC "ls -la /home/travis"
echo "ls -la /home/travis/build"
DOCKER_EXEC "ls -la /home/travis/build"
echo "ls -la /home/travis/build/project-liberty/"
DOCKER_EXEC "ls -la /home/travis/build/project-liberty/"

BEGIN_FOLD build
    DOCKER_EXEC "su travis -c 'make -C /home/travis/build/project-liberty/wallet/depends $MAKEJOBS $GOAL'"
    #DOCKER_EXEC "su travis -c 'make $MAKEJOBS'"
    DOCKER_EXEC "su travis -c 'make $GOAL V=1 ; false )'"
END_FOLD

if [ "$RUN_UNIT_TESTS" = "true" ]; then
  BEGIN_FOLD unit-tests
  #DOCKER_EXEC LD_LIBRARY_PATH=$TRAVIS_BUILD_DIR/depends/$HOST/lib make $MAKEJOBS check VERBOSE=1
  END_FOLD
fi

if [ "$RUN_BENCH" = "true" ]; then
  BEGIN_FOLD bench
  #DOCKER_EXEC LD_LIBRARY_PATH=$TRAVIS_BUILD_DIR/depends/$HOST/lib $OUTDIR/bin/bench_liberty -scaling=0.001
  END_FOLD
fi

if [ "$TRAVIS_EVENT_TYPE" = "cron" ]; then
  extended="--extended --exclude feature_pruning,feature_dbcrash"
fi

if [ "$RUN_FUNCTIONAL_TESTS" = "true" ]; then
  BEGIN_FOLD functional-tests
  #DOCKER_EXEC test/functional/test_runner.py --combinedlogslen=4000 --coverage --quiet --failfast ${extended}
  END_FOLD
fi
