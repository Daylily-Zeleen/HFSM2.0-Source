#!/bin/bash

if [ ! -e "./misc/scripts/make_tarball.sh" ]; then
  echo "This script should be ran from the root folder."
  exit 1
fi

archive_prefix="../hfsm2"

while getopts "h?p:" opt; do
  case "$opt" in
  h|\?)
    echo "Usage: $0 [OPTIONS...]"
    echo
    echo "  -p archive_prefix, default \"../hfsm2\", it means that create acrhive at work folder's parent folder with name \"hfsm2.tar\"."
    echo
    exit 1
    ;;
  p)
    archive_prefix=$OPTARG
    ;;
  esac
done

archive_tmp_dir=".tmp"
archive_name="${archive_prefix}.tar"

if [ -d ${archive_tmp_dir} ]; then
  rm -rf ${archive_tmp_dir}
fi

mkdir ${archive_tmp_dir}
pwd=$(pwd)

git archive --format=tar HEAD | tar xf - --directory ${archive_tmp_dir} # (cd ${archive_tmp_dir} && tar xf -) #归档父项目后解压到指定目录
git submodule foreach | while read desc subdir; do
  echo Handling submodule $subdir
  subdir=${subdir#*\'} #去除最左边的单引号
  subdir=${subdir%*\'} #去除最右边的单引号
  [ "${subdir}" = "" ] && continue
  # 加一步判断，subdir为""则continue

  pushd ${subdir}
  git archive --format=tar HEAD | tar xf - --directory ${pwd}/${archive_tmp_dir}/${subdir} #归档submodule后解压到父目录
  popd
done

if [ -e ${archive_name} ]; then
  rm -f ${archive_name}
fi

tar -uf ${archive_name} --directory ${archive_tmp_dir} `ls -A ${archive_tmp_dir}`

echo "Make tarball done! Archive file: \"${archive_name}\""

rm -rf ${archive_tmp_dir}
