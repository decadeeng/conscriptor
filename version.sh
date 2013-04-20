#! /bin/sh
echo "const char *gitrev = \"`git log |grep commit |sed s/commit\ // |head -c5`\";" > conscriptor/version.h
echo "const int revision = `git log |grep commit |wc -l`;" >> conscriptor/version.h
