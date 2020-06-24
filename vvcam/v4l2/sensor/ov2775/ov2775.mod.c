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
	{ 0x88b23bfb, "regulator_set_voltage" },
	{ 0x815588a6, "clk_enable" },
	{ 0x9fc847e0, "i2c_del_driver" },
	{ 0x61b2f0ef, "regulator_disable" },
	{ 0x74435d4e, "pinctrl_select_state" },
	{ 0xb6e6d99d, "clk_disable" },
	{ 0x5a0c2c7c, "devm_pinctrl_get" },
	{ 0xda847031, "gpio_to_desc" },
	{ 0xeae3dfd6, "__const_udelay" },
	{ 0xd9bd2344, "pinctrl_lookup_state" },
	{ 0xc39a9b6c, "v4l2_device_register" },
	{ 0xef3a37e1, "v4l2_device_disconnect" },
	{ 0x4088410e, "_dev_warn" },
	{ 0xdcb764ad, "memset" },
	{ 0xc5850110, "printk" },
	{ 0x2a69e4, "media_entity_pads_init" },
	{ 0x7de09ea0, "_dev_err" },
	{ 0x7a9b9cc9, "devm_gpio_request_one" },
	{ 0xf587b1a3, "v4l2_device_unregister_subdev" },
	{ 0x48715993, "i2c_register_driver" },
	{ 0x95a396b3, "kmem_cache_alloc" },
	{ 0xb89ed56a, "devm_regulator_get" },
	{ 0xdecd0b29, "__stack_chk_fail" },
	{ 0x7c9a7371, "clk_prepare" },
	{ 0xd5945d00, "of_get_named_gpio_flags" },
	{ 0x8a2c9162, "v4l2_device_register_subdev" },
	{ 0xd34c21f3, "devm_clk_get" },
	{ 0x76d9b876, "clk_set_rate" },
	{ 0x24f1915, "i2c_transfer_buffer_flags" },
	{ 0xe5f350ed, "v4l2_device_register_subdev_nodes" },
	{ 0xd07e5be4, "devm_pinctrl_put" },
	{ 0x49064452, "gpiod_set_raw_value_cansleep" },
	{ 0xfaf8a927, "of_property_read_variable_u32_array" },
	{ 0xc01d056a, "devm_kmalloc" },
	{ 0x1ed3f8d6, "v4l2_device_put" },
	{ 0xbe036250, "v4l2_i2c_subdev_init" },
	{ 0x2bbc5475, "regulator_enable" },
};

MODULE_INFO(depends, "");

MODULE_ALIAS("i2c:ov2775");
MODULE_ALIAS("of:N*T*Covti,ov2775");
MODULE_ALIAS("of:N*T*Covti,ov2775C*");
