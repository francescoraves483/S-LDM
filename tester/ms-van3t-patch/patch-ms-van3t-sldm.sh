#!/bin/bash

echo "Patching..."
git pull
git apply 0001-patch-ms-van3t-sldm.patch
mv metadata-header.* ./src/automotive/model/GeoNet
echo "Ok. Script terminated."

echo "Self-destruct activated!"
rm 0001-patch-ms-van3t-sldm.patch
rm patch-ms-van3t-sldm.sh
echo "Kaboom! Done!"
