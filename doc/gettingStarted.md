# Getting Started

## Install

```sh 
git clone https://github.com/WorkhorseComputing/WorkhorseRT
cd WorkhorseRT
```

package lists can be found in scripts/install, architecture specific package lists can be found in arch/**/scripts/install

## Building WorkhorseRT

WorkhorseRT can be built using cmake

```sh
bash config.sh genconfig
mkdir build
cd build
cmake ..
make
```

## Testing

WorkhorseRT can be tested in QEMU if supported:

```sh
make run
```

Using KVM:

```sh
make runKvm
```