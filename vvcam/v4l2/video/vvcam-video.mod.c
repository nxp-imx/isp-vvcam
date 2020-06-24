#include <linux/build-salt.h>
#include <linux/module.h>
#include <linux/vermagic.h>
#include <linux/compiler.h>

BUILD_SALT;

MODULE_INFO(vermagic, VERMAGIC_STRING);
MODULE_INFO(name, KBUILD_MODNAME);

__visible struct module __this_module
__section(.gnu.linkonce.this_module) = {
	.name = KBUILD_MODNAME,
	.init = init_module,
#ifdef CONFIG_MODULE_UNLOAD
	.exit = cleanup_module,
#endif
	.arch = MODULE_ARCH_INIT,
};

#ifdef CONFIG_RETPOLINE
MODULE_INFO(retpoline, "Y");
#endif

static const struct modversion_info ____versions[]
__used __section(__versions) = {
	{ 0xd17348c1, "module_layout" },
	{ 0x2391bc01, "kmalloc_caches" },
	{ 0x1fc3c58d, "v4l2_event_unsubscribe" },
	{ 0xf9a482f9, "msleep" },
	{ 0xa585d3f8, "v4l2_event_queue" },
	{ 0xa8788b5b, "vb2_mmap" },
	{ 0x45e94bb3, "video_device_release" },
	{ 0xb43f9365, "ktime_get" },
	{ 0x8cb7a714, "__video_register_device" },
	{ 0x409bcb62, "mutex_unlock" },
	{ 0x7b1c6fd9, "__platform_driver_register" },
	{ 0xc39a9b6c, "v4l2_device_register" },
	{ 0xd9a5ea54, "__init_waitqueue_head" },
	{ 0xef3a37e1, "v4l2_device_disconnect" },
	{ 0xdcb764ad, "memset" },
	{ 0x13b8bba, "video_device_alloc" },
	{ 0x3812050a, "_raw_spin_unlock_irqrestore" },
	{ 0xd82f25a2, "v4l2_event_subscribe" },
	{ 0x977f511b, "__mutex_init" },
	{ 0xc5850110, "printk" },
	{ 0x23b62a2c, "video_unregister_device" },
	{ 0xc45ad627, "v4l2_fh_init" },
	{ 0x4bba47a2, "v4l2_event_pending" },
	{ 0xfbc68ed4, "vb2_buffer_done" },
	{ 0x5792f848, "strlcpy" },
	{ 0x2ab7989d, "mutex_lock" },
	{ 0xae282021, "vb2_qbuf" },
	{ 0x75f5b979, "platform_device_unregister" },
	{ 0xc1e58a5f, "refcount_dec_and_test_checked" },
	{ 0x2926d27f, "vb2_querybuf" },
	{ 0x7de09ea0, "_dev_err" },
	{ 0x95a396b3, "kmem_cache_alloc" },
	{ 0x736a29bb, "vb2_streamon" },
	{ 0xf5ef842e, "v4l_bound_align_image" },
	{ 0x76d28ff0, "platform_device_register" },
	{ 0x7b144583, "put_device" },
	{ 0x70fccb42, "video_devdata" },
	{ 0xdecd0b29, "__stack_chk_fail" },
	{ 0xfb285df9, "vb2_reqbufs" },
	{ 0x51760917, "_raw_spin_lock_irqsave" },
	{ 0x9bc909e2, "get_device" },
	{ 0x7970ef0b, "vb2_dqbuf" },
	{ 0xe5f350ed, "v4l2_device_register_subdev_nodes" },
	{ 0x37a0cba, "kfree" },
	{ 0xd23daedd, "remap_pfn_range" },
	{ 0x4829a47e, "memcpy" },
	{ 0x1de9566a, "vb2_common_vm_ops" },
	{ 0x7a4497db, "kzfree" },
	{ 0x641bb1ec, "v4l2_fh_add" },
	{ 0x83035215, "v4l2_fh_del" },
	{ 0x29361773, "complete" },
	{ 0xd68294bd, "vb2_poll" },
	{ 0xbc2d8efd, "platform_driver_unregister" },
	{ 0x4d1ff60a, "wait_for_completion_timeout" },
	{ 0x7e72db25, "vb2_queue_release" },
	{ 0x1ed3f8d6, "v4l2_device_put" },
	{ 0xfb432bd4, "vb2_streamoff" },
	{ 0x7aea312f, "video_ioctl2" },
	{ 0xa184bd4e, "v4l2_fh_exit" },
	{ 0x5c6f335c, "vb2_queue_init" },
};

MODULE_INFO(depends, "");

