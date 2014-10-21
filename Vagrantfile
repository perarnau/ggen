# -*- mode: ruby -*-
# vi: set ft=ruby :

# Vagrant config for a ggen install under Ubuntu 14.04 LTS 64bit.
# The script doesn't cleanup, for fear of running rm -rf on a shared dir.
# Use either from the ggen dir or a directory above it.
# You don't need a copy of the ggen repository for this to work

$bootstrap = <<SCRIPT
echo "installing additional system packages"
apt-get update
apt-get install -y git libgsl0-dev libgraphviz-dev pandoc g++ libxml2-dev \
		automake pkg-config
echo "installing igraph"
wget -nv http://igraph.org/nightly/get/c/igraph-0.7.1.tar.gz
tar xzf igraph-0.7.1.tar.gz
cd igraph-0.7.1
./configure --prefix=/usr
make
make install
cd ..
echo "installing ggen"
git clone https://github.com/perarnau/ggen.git
cd ggen
./autogen.sh
./configure --prefix=/usr
make
make check
make install
cd ..
echo "cleaning up"
rm -rf ggen igraph-0.7.1*
echo "done"
SCRIPT

# Vagrantfile API/syntax version. Don't touch unless you know what you're doing!
VAGRANTFILE_API_VERSION = "2"

Vagrant.configure(VAGRANTFILE_API_VERSION) do |config|
  config.vm.box = "ubuntu/trusty64"
  config.vm.provision :shell, inline: $bootstrap
  config.vm.network "forwarded_port", guest: 8888, host: 8888
end
