#!/usr/bin/env bash
#
# Copyright (c) 2018 The Bitcoin Core developers
# Distributed under the MIT software license, see the accompanying
# file COPYING or http://www.opensource.org/licenses/mit-license.php.

export LC_ALL=C.UTF-8

TRAVIS_COMMIT_LOG=$(git log --format=fuller -1)
export TRAVIS_COMMIT_LOG

echo "list all users on docker?"
DOCKER_EXEC cut -d: -f1 /etc/passwd

echo "DOCKER_EXEC pwd"
DOCKER_EXEC pwd
ls -la

OUTDIR=$BASE_OUTDIR/$TRAVIS_PULL_REQUEST/$TRAVIS_JOB_NUMBER-$HOST
BITCOIN_CONFIG_ALL="--disable-dependency-tracking --prefix=$TRAVIS_BUILD_DIR/depends/$HOST --bindir=$OUTDIR/bin --libdir=$OUTDIR/lib"
if [ -z "$NO_DEPENDS" ]; then
  DOCKER_EXEC ccache --max-size=$CCACHE_SIZE
fi

BEGIN_FOLD autogen
if [ -n "$CONFIG_SHELL" ]; then
  echo "CONFIG_SHELL -c autogen.sh before"
  DOCKER_EXEC $CONFIG_SHELL -c ./autogen.sh
  echo "CONFIG_SHELL -c autogen.sh after"
else
  echo "else autogen.sh before"
  #DOCKER_EXEC "su -c travis './autogen.sh'"
  DOCKER_EXEC "sudo -u \#1000 './autogen.sh'"
  echo "else autogen.sh after"

fi
END_FOLD

pwd
echo "ls -la before cd build"
ls -la
#DOCKER_EXEC "sudo -u \#1000 'mkdir build'"
cd build || (echo "could not enter build directory"; exit 1)

pwd
echo "ls -la after cd build"
ls -la

whoami
#sudo chown -R travis:travis /home/travis

BEGIN_FOLD configure
echo "first configure begin"
   # DOCKER_EXEC ../configure --cache-file=config.cache $BITCOIN_CONFIG_ALL $BITCOIN_CONFIG || ( cat config.log && false) && make VERSION=$HOST
  #DOCKER_EXEC "sudo -u \#1000 '../configure --cache-file=config.cache $BITCOIN_CONFIG_ALL $BITCOIN_CONFIG || ( cat config.log && false)'"
  DOCKER_EXEC "sudo -u \#1000 '../configure --cache-file=config.cache $BITCOIN_CONFIG_ALL $BITCOIN_CONFIG'"
  echo "first configure end"
END_FOLD

echo "next is make VERSION=$HOST" 


BEGIN_FOLD distdir
#DOCKER_EXEC make VERSION=$HOST
echo "make VERSION=$HOST begin"
   #DOCKER_EXEC "make VERSION=$HOST"
  DOCKER_EXEC "sudo -u \#1000 'make VERSION=$HOST'"
   echo "make VERSION=$HOST end"
END_FOLD

DOCKER_EXEC cd "liberty-$HOST" || (echo "could not enter distdir liberty-$HOST"; exit 1)

pwd
echo "ls -la before second configure"
ls -la

BEGIN_FOLD configure
   echo "second configure begin"
   # DOCKER_EXEC CONFIG_SHELL= ./configure --cache-file=../config.cache $BITCOIN_CONFIG_ALL $BITCOIN_CONFIG || ( cat config.log && false) 
   DOCKER_EXEC "sudo -u \#1000 './configure --cache-file=../config.cache $BITCOIN_CONFIG_ALL $BITCOIN_CONFIG'" #|| ( cat config.log && false)
   echo "second configure begin"
END_FOLD


BEGIN_FOLD build
echo "build begin"
   DOCKER_EXEC "sudo -u \#1000  'make $MAKEJOBS $GOAL'" 
   #|| ( echo "Build failure. Verbose build follows." && DOCKER_EXEC "su -c travis -s make $GOAL V=1 ; false" )'"
   echo "build end"
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
