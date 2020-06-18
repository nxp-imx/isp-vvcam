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
	{ 0x12a4e128, "__arch_copy_from_user" },
	{ 0x7b1c6fd9, "__platform_driver_register" },
	{ 0xc39a9b6c, "v4l2_device_register" },
	{ 0xef3a37e1, "v4l2_device_disconnect" },
	{ 0xdcb764ad, "memset" },
	{ 0xd82f25a2, "v4l2_event_subscribe" },
	{ 0xc5850110, "printk" },
	{ 0x75f5b979, "platform_device_unregister" },
	{ 0x6b4b2933, "__ioremap" },
	{ 0xf587b1a3, "v4l2_device_unregister_subdev" },
	{ 0x95a396b3, "kmem_cache_alloc" },
	{ 0x76d28ff0, "platform_device_register" },
	{ 0x6cbbfc54, "__arch_copy_to_user" },
	{ 0xdecd0b29, "__stack_chk_fail" },
	{ 0xb342df7a, "cpu_hwcaps" },
	{ 0x8a2c9162, "v4l2_device_register_subdev" },
	{ 0x1333db27, "cpu_hwcap_keys" },
	{ 0x9ebbdf13, "v4l2_subdev_init" },
	{ 0xe5f350ed, "v4l2_device_register_subdev_nodes" },
	{ 0xedc03953, "iounmap" },
	{ 0x7a4497db, "kzfree" },
	{ 0xbc2d8efd, "platform_driver_unregister" },
	{ 0xb2ead97c, "kimage_vaddr" },
	{ 0x1ed3f8d6, "v4l2_device_put" },
	{ 0x14b89635, "arm64_const_caps_ready" },
};

MODULE_INFO(depends, "");


MODULE_INFO(srcversion, "3C1131C3251179F9521DAE8");
