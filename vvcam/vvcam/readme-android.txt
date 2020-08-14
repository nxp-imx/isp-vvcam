To build under android top dir, please follow below steps.
1. clone the git under ANDROID_ROOT/vendor/nxp-opensource
2. check out to branch integration_vsi_4.0.8p2_android
3. cd ANDROID_ROOT
4. source build/envsetup.sh
5. lunch evk_8mp-userdebug
6. ./imx-make.sh kernel -j8
7. ./imx-make.sh vvcam  (add "-c" for clean build)
8. generate the ko in ANDROID_ROOT/out/target/product/evk_8mp/obj/VVCAM_OBJ/vvcam.ko
