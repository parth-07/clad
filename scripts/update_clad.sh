clad_home=/workspace/builds/build-${1}

if [[ ! -d ${clad_home} ]]
then 
  echo "Clad home do not exist!"
  echo "Configuring and building Clad."
  build_clad.sh ${1}
else
  echo "Building Clad."
  cd ${clad_home}/obj
  ninja install
fi
