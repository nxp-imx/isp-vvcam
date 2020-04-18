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
	{ 0xb077e70a, "clk_unprepare" },
	{ 0x939f9495, "kmalloc_caches" },
	{ 0x1fc3c58d, "v4l2_event_unsubscribe" },
	{ 0xf9a482f9, "msleep" },
	{ 0xbd486fa9, "__pm_runtime_idle" },
	{ 0x1d22eaaa, "regulator_set_voltage" },
	{ 0x815588a6, "clk_enable" },
	{ 0x7f99f00e, "__pm_runtime_disable" },
	{ 0x3b3d4836, "param_ops_int" },
	{ 0xa585d3f8, "v4l2_event_queue" },
	{ 0x9fc847e0, "i2c_del_driver" },
	{ 0xa8788b5b, "vb2_mmap" },
	{ 0x59e9d6a6, "v4l2_subdev_call_wrappers" },
	{ 0xfc7f7082, "regulator_disable" },
	{ 0x45e94bb3, "video_device_release" },
	{ 0x2d306308, "regmap_update_bits_base" },
	{ 0x74435d4e, "pinctrl_select_state" },
	{ 0xb6e6d99d, "clk_disable" },
	{ 0x80868a4f, "of_alias_get_id" },
	{ 0x12a4e128, "__arch_copy_from_user" },
	{ 0x5a0c2c7c, "devm_pinctrl_get" },
	{ 0xda847031, "gpio_to_desc" },
	{ 0x56470118, "__warn_printk" },
	{ 0xb43f9365, "ktime_get" },
	{ 0xeae3dfd6, "__const_udelay" },
	{ 0x8cb7a714, "__video_register_device" },
	{ 0x409bcb62, "mutex_unlock" },
	{ 0xfedeceda, "__pm_runtime_resume" },
	{ 0xfdf41c63, "regmap_read" },
	{ 0xd9bd2344, "pinctrl_lookup_state" },
	{ 0x7b1c6fd9, "__platform_driver_register" },
	{ 0xc39a9b6c, "v4l2_device_register" },
	{ 0xd9a5ea54, "__init_waitqueue_head" },
	{ 0xef3a37e1, "v4l2_device_disconnect" },
	{ 0x25966f20, "_dev_warn" },
	{ 0xdcb764ad, "memset" },
	{ 0x13b8bba, "video_device_alloc" },
	{ 0x3812050a, "_raw_spin_unlock_irqrestore" },
	{ 0xd82f25a2, "v4l2_event_subscribe" },
	{ 0x977f511b, "__mutex_init" },
	{ 0xc5850110, "printk" },
	{ 0x35cd6896, "of_match_node" },
	{ 0x23b62a2c, "video_unregister_device" },
	{ 0xf7ca9402, "of_graph_get_next_endpoint" },
	{ 0x2a69e4, "media_entity_pads_init" },
	{ 0xbf49b1d4, "of_find_property" },
	{ 0xc45ad627, "v4l2_fh_init" },
	{ 0x4bba47a2, "v4l2_event_pending" },
	{ 0xfbc68ed4, "vb2_buffer_done" },
	{ 0x5792f848, "strlcpy" },
	{ 0x2ab7989d, "mutex_lock" },
	{ 0xae282021, "vb2_qbuf" },
	{ 0x84b04cc3, "platform_get_resource" },
	{ 0x75f5b979, "platform_device_unregister" },
	{ 0xc1e58a5f, "refcount_dec_and_test_checked" },
	{ 0x6b4b2933, "__ioremap" },
	{ 0x2926d27f, "vb2_querybuf" },
	{ 0xc7e5ef81, "_dev_err" },
	{ 0x7a9b9cc9, "devm_gpio_request_one" },
	{ 0xf587b1a3, "v4l2_device_unregister_subdev" },
	{ 0x48715993, "i2c_register_driver" },
	{ 0xae9a75b5, "_dev_info" },
	{ 0x6bf39bf5, "kmem_cache_alloc" },
	{ 0x543e3c8, "devm_regulator_get" },
	{ 0x736a29bb, "vb2_streamon" },
	{ 0xf5ef842e, "v4l_bound_align_image" },
	{ 0x76d28ff0, "platform_device_register" },
	{ 0xfb76f787, "pm_runtime_enable" },
	{ 0xe47012ab, "syscon_regmap_lookup_by_phandle" },
	{ 0x5edbed73, "put_device" },
	{ 0x6cbbfc54, "__arch_copy_to_user" },
	{ 0x70fccb42, "video_devdata" },
	{ 0xdecd0b29, "__stack_chk_fail" },
	{ 0xb342df7a, "cpu_hwcaps" },
	{ 0x7c9a7371, "clk_prepare" },
	{ 0xaf3e6a8a, "of_find_compatible_node" },
	{ 0xd5945d00, "of_get_named_gpio_flags" },
	{ 0x8a2c9162, "v4l2_device_register_subdev" },
	{ 0x1333db27, "cpu_hwcap_keys" },
	{ 0xa2855876, "dev_driver_string" },
	{ 0xfb285df9, "vb2_reqbufs" },
	{ 0xd34c21f3, "devm_clk_get" },
	{ 0x51760917, "_raw_spin_lock_irqsave" },
	{ 0x3df32c65, "get_device" },
	{ 0x76d9b876, "clk_set_rate" },
	{ 0x9ebbdf13, "v4l2_subdev_init" },
	{ 0x24f1915, "i2c_transfer_buffer_flags" },
	{ 0xfbe4687a, "media_entity_remote_pad" },
	{ 0x7970ef0b, "vb2_dqbuf" },
	{ 0xe5f350ed, "v4l2_device_register_subdev_nodes" },
	{ 0xd07e5be4, "devm_pinctrl_put" },
	{ 0x37a0cba, "kfree" },
	{ 0x353b1ece, "remap_pfn_range" },
	{ 0x4829a47e, "memcpy" },
	{ 0x1de9566a, "vb2_common_vm_ops" },
	{ 0xedc03953, "iounmap" },
	{ 0x7a4497db, "kzfree" },
	{ 0x641bb1ec, "v4l2_fh_add" },
	{ 0x49064452, "gpiod_set_raw_value_cansleep" },
	{ 0x83035215, "v4l2_fh_del" },
	{ 0x29361773, "complete" },
	{ 0x656e4a6e, "snprintf" },
	{ 0xd68294bd, "vb2_poll" },
	{ 0xbc2d8efd, "platform_driver_unregister" },
	{ 0xfaf8a927, "of_property_read_variable_u32_array" },
	{ 0x30891bbf, "of_node_put" },
	{ 0xc01d056a, "devm_kmalloc" },
	{ 0x4d1ff60a, "wait_for_completion_timeout" },
	{ 0x7e72db25, "vb2_queue_release" },
	{ 0xb2ead97c, "kimage_vaddr" },
	{ 0x1ed3f8d6, "v4l2_device_put" },
	{ 0x14b89635, "arm64_const_caps_ready" },
	{ 0xfb432bd4, "vb2_streamoff" },
	{ 0xbd4b4e11, "regmap_write" },
	{ 0x7aea312f, "video_ioctl2" },
	{ 0xbe036250, "v4l2_i2c_subdev_init" },
	{ 0xa66a3fed, "regulator_enable" },
	{ 0xa184bd4e, "v4l2_fh_exit" },
	{ 0x5c6f335c, "vb2_queue_init" },
};

MODULE_INFO(depends, "");

MODULE_ALIAS("i2c:ov2775");
MODULE_ALIAS("of:N*T*Cfsl,imx8mn-mipi-csi");
MODULE_ALIAS("of:N*T*Cfsl,imx8mn-mipi-csiC*");
MODULE_ALIAS("i2c:ov2775");
