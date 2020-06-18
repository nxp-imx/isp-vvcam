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
	{ 0x2391bc01, "kmalloc_caches" },
	{ 0xf9a482f9, "msleep" },
	{ 0xbd486fa9, "__pm_runtime_idle" },
	{ 0x19349726, "pm_runtime_force_suspend" },
	{ 0x88b23bfb, "regulator_set_voltage" },
	{ 0x815588a6, "clk_enable" },
	{ 0x3b3d4836, "param_ops_int" },
	{ 0x59e9d6a6, "v4l2_subdev_call_wrappers" },
	{ 0x61b2f0ef, "regulator_disable" },
	{ 0x2d306308, "regmap_update_bits_base" },
	{ 0xb6e6d99d, "clk_disable" },
	{ 0x80868a4f, "of_alias_get_id" },
	{ 0x56470118, "__warn_printk" },
	{ 0xbdfb6ba, "pm_runtime_force_resume" },
	{ 0xeae3dfd6, "__const_udelay" },
	{ 0x409bcb62, "mutex_unlock" },
	{ 0xfedeceda, "__pm_runtime_resume" },
	{ 0xfdf41c63, "regmap_read" },
	{ 0x7b1c6fd9, "__platform_driver_register" },
	{ 0xc39a9b6c, "v4l2_device_register" },
	{ 0xef3a37e1, "v4l2_device_disconnect" },
	{ 0x3812050a, "_raw_spin_unlock_irqrestore" },
	{ 0x977f511b, "__mutex_init" },
	{ 0xc5850110, "printk" },
	{ 0x35cd6896, "of_match_node" },
	{ 0xf7ca9402, "of_graph_get_next_endpoint" },
	{ 0x2a69e4, "media_entity_pads_init" },
	{ 0xbf49b1d4, "of_find_property" },
	{ 0x2ab7989d, "mutex_lock" },
	{ 0x84b04cc3, "platform_get_resource" },
	{ 0x6b4b2933, "__ioremap" },
	{ 0x7de09ea0, "_dev_err" },
	{ 0xf587b1a3, "v4l2_device_unregister_subdev" },
	{ 0xa09de553, "_dev_info" },
	{ 0x95a396b3, "kmem_cache_alloc" },
	{ 0xb89ed56a, "devm_regulator_get" },
	{ 0xfb76f787, "pm_runtime_enable" },
	{ 0xe47012ab, "syscon_regmap_lookup_by_phandle" },
	{ 0xdecd0b29, "__stack_chk_fail" },
	{ 0x64a4d318, "of_clk_set_defaults" },
	{ 0xb342df7a, "cpu_hwcaps" },
	{ 0x7c9a7371, "clk_prepare" },
	{ 0xaf3e6a8a, "of_find_compatible_node" },
	{ 0x8a2c9162, "v4l2_device_register_subdev" },
	{ 0x1333db27, "cpu_hwcap_keys" },
	{ 0x28b83057, "dev_driver_string" },
	{ 0xd34c21f3, "devm_clk_get" },
	{ 0x51760917, "_raw_spin_lock_irqsave" },
	{ 0x76d9b876, "clk_set_rate" },
	{ 0x9ebbdf13, "v4l2_subdev_init" },
	{ 0xfbe4687a, "media_entity_remote_pad" },
	{ 0xe5f350ed, "v4l2_device_register_subdev_nodes" },
	{ 0x4829a47e, "memcpy" },
	{ 0x656e4a6e, "snprintf" },
	{ 0xbc2d8efd, "platform_driver_unregister" },
	{ 0xfaf8a927, "of_property_read_variable_u32_array" },
	{ 0x30891bbf, "of_node_put" },
	{ 0xc01d056a, "devm_kmalloc" },
	{ 0xb2ead97c, "kimage_vaddr" },
	{ 0x1ed3f8d6, "v4l2_device_put" },
	{ 0x14b89635, "arm64_const_caps_ready" },
	{ 0xbd4b4e11, "regmap_write" },
	{ 0x2bbc5475, "regulator_enable" },
};

MODULE_INFO(depends, "");

MODULE_ALIAS("of:N*T*Cfsl,imx8mn-mipi-csi");
MODULE_ALIAS("of:N*T*Cfsl,imx8mn-mipi-csiC*");

MODULE_INFO(srcversion, "612B6CA51F4952D340FD5BF");
