#!/bin/bash
# Install SMARTLING demo-site.
read -n 1 -p "Are you sure you want to run the installation Smartling demo-site (y/n): " AMSURE
[ "$AMSURE" = "y" ] || exit
echo "" 1>&2

MINPARAMS=4

if [ -n "$1" ]
then
  user=$1
  echo "user = $user"
else
  echo='You did not enter the parameter USER'
fi

if [ -n "$2" ]
then
  pass=$2
  echo "pass = $pass"
else
  echo='You did not enter the parameter PASSWORD'
fi

if [ -n "$3" ]
then
  host=$3
  echo "host = $host"
else
  echo='You did not enter the parameter HOST'
fi

if [ -n "$4" ]
then
  db_name=$4
  echo "db_name = $db_name"
else
  echo='You did not enter the parameter DB_NAME'
fi

if [ $# -lt "$MINPARAMS" ]
then
  echo "The number of command line arguments should be at least $MINPARAMS !"
fi

git clone https://github.com/Smartling/drupal-localization-module.git

mv drupal-localization-module/smartling-demo-install.make smartling-demo-install.make

echo='Start drush make'
drush make smartling-demo-install.make -y
echo='OK'

echo='Start drush site-install'
drush site-install standard --db-url=mysql://${user}:${pass}@${host}/${db_name} --account-name=admin --account-pass=admin --site-name=Smartling -y

mkdir sites/all/modules/custom/
mv drupal-localization-module/smartling sites/all/modules/custom/

git clone https://github.com/Smartling/api-sdk-php.git sites/all/modules/custom/smartling/api
chmod -R 777 sites/default/files

drush en admin_menu ultimate_cron -y
drush dis overlay toolbar -y
drush vset --exact ultimate_cron_poorman 0
drush en smartling -y
drush en smartling_demo_content -y
drush fra -y
drush cc all -y
drush cron-run defaultcontent_cron
drush cron
echo='OK'

exit 0
