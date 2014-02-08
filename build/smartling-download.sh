#!/bin/bash
#
# This helper script do download smartling module and zip it
#
################################

logit () {
    DATE=$(date +"[%Y-%m-%d %H:%M:%S]")
    echo -n "$DATE "
    case $1 in
        info) echo -n '[INFO] '
            ;;
        warn) echo -n '[WARNING] '
            ;;
        err)  echo -n '[ERROR] '
            ;;
        *) echo $1
            ;;
    esac
    echo $2
}

# git url
GIT=$(which git)
if [ "x$GIT" = "x" ]; then
  logit err "Git not found in PATH. Exiting"
  exit 1
fi

# zip
ZIP=$(which zip)
if [ "x$ZIP" = "x" ]; then
  logit err "Zip not found in PATH. Exiting"
  exit 1
fi

# read options
set -- $(getopt -n$0 -u -a --longoptions="tag: " "h" "$@") || usage

while [ $# -gt 0 ];do
    case "$1" in
        --tag) TAG=$2;shift;;
        -h)     usage;;
        --)     shift;break;;
        -*)     usage;;
        *)      break;;
    esac
    shift
done

# check params
[ "x$TAG" == "x" ] && { logit err "-tag option should be set"; usage; }

# search for git executable
logit info "Download source from repo"
$GIT clone https://github.com/Smartling/drupal-localization-module.git smartling
$GIT clone https://github.com/Smartling/api-sdk-php.git smartling/smartling/api
RESULT=$?
[ $RESULT -ne 0 ] && { logit err "Git error $RESULT. Exiting"; exit $RESULT; }
logit info "Done"

logit info "Archiving"

cd smartling
$GIT checkout tags/$TAG

rm -R smartling/api/.git

$ZIP  -y -r -q ../smartling.zip smartling/
cd ../
rm -R smartling/

logit info "Download done"
exit 0 
