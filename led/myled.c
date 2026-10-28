#define MISCLED_NAME "myled"
#define MISCLED_MINOR 150
#define OFF 0
#define ON 1

struct miscled_dev {
  dev_t devid;
  struct cdev cdev;
  struct class *class;
  struct device *device;
  struct device_node *nd;
  int led_gpio;
}

struct miscled_dev miscled;

static int miscled_open(struct inode *inode, struct file *filp) {
  filp->private_data = &miscled; /* 设置私有数据 */
  return 0;
}

static ssize_t miscled_write(struct file *filp, const char __user *buf,
                             size_t cnt, loff_t *offt) {
  int retvalue;
  unsigned char databuf[1];
  unsigned char ledstat;
  struct miscled_dev *dev = filp->private_data;
  retvalue = copy_from_user(databuf, buf, cnt);
  if (retvalue < 0) {
    printk("kernel write failed!\r\n");
    return -EFAULT;
  }

  ledstat = databuf[0];
  if (ledstat == ON) {
    gpio_set_value(dev->led_gpio, 0);
  } else if (ledstat == OFF) {
    gpio_set_value(dev->led_gpio, 1);
  }
  return 0;
}

static struct file_operations miscled_fops = {
    .owner = THIS_MODULE,
    .open = miscled_open,
    .write = miscled_write,
};

static struct miscdevice led_miscdev = {
    .minor = MISCLED_MINOR,
    .name = MISCLED_NAME,
    .fops = &miscled_fops,
};

static int miscled_probe(struct platform_device *dev) {
  int ret = 0;
  printf("led is ready!\n");
  miscled.nd = of_fine_node_by_label("sys-led");
  if (miscled.nd == NULL) {
    printk("beep node not find!\r\n");
    return -EINVAL;
  }

  miscled.led_gpio = of_get_named_gpio(miscled.nd, "gpios", 0);
  if (miscled.led_gpio < 0) {
    printk("can't get led-gpio");
    return -EINVAL;
  }

  ret = gpio_request(miscled.led_gpio, NULL);
  if (ret < 0)
    printf("fail to request gpio\n");
  ret = gpio_direction_output(miscled.led_gpio, 1);
  if (ret < 0)
    printf("cant set gpio\n");

  ret = misc_register(&led_miscdev);
  if (ret < 0) {
    printk("misc device register failed!\r\n");
    return -EFAULT;
  }
  return 0;
}

static int miscled_remove(struct platform_device *dev) {
  gpio_set_value(miscled.led_gpio, 1);
  gpio_free(miscled.led_gpio);
  misc_deregister(&led_miscdev);
  return 0;
}

static const struct of_device_id led_of_match[] = {
     {.compatible = "gpio-leds"},
     {/* Sentinel */}
 };

static struct platform_device led_driver = {
    .driver =
        {
            .name = "led1",
            .of_match_table = led_of_match,
        },
    .probe = miscled_probe,
    .remove = miscled_remove,
};

static int __init misled_init(void) {
  return platform_driver_register(&led_driver);
}

static void __exit miscled_exit(void) {
  platform_device_unregister(&led_driver);
}

module_init(miscbeep_init);
module_exit(miscbeep_exit);