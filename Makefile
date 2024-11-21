KDIR ?= /lib/modules/$(shell uname -r)/build
SRC = src/process-and-memory.c
TESTER_SRC = src/tester/main.c
KERNEL_OUT ?= $(KDIR)/System.map $(KDIR)/arch/x86/boot/bzImage $(KDIR)/.config

# Folders
SRC_DIR = src
OUTS = obj

SRC = tester/main.c
SRC_PLUS_PATH = $(addprefix $(SRC_DIR)/, $(SRC))
OUT = $(subst $(SRC_DIR)/, $(OUTS)/, $(patsubst %.c, %.o, $(SRC_PLUS_PATH)))

NAME = ps_tester

$(NAME): $(OUT) src/tester/main.c
	@gcc $(OUT) -o $(NAME) -Wall -Wextra -Werror

$(OUT): $(OUTS)/%.o : $(SRC_DIR)/%.c
	@echo "Compiling $(basename $(notdir $*.o))"
	@mkdir -p $(@D)
	@gcc -Wall -Werror -Wextra -g -c $< -o $@

clean:
	@echo "Cleaning object files"
	@rm -rf obj

fclean: clean
	@echo "Cleaning binary"
	@rm -rf ps_tester

re: fclean
	@$(MAKE) -C $$PWD

patch_kernel: patch.diff
	git -C $(KDIR) apply $$PWD/patch.diff
	touch patch_kernel

restore_kernel: patch_kernel
	git -C $(KDIR) checkout .
	rm -rf $(KDIR)/kernel/get_pid_info
	rm -rf patch_kernel

mount_boot:
	mount /dev/vda1

compile_kernel: patch_kernel
	$(MAKE) -C $(KDIR) LLVM=1 -j 6

install_kernel: mount_boot compile_kernel
	$(MAKE) -C $(KDIR) LLVM=1 modules_install
	cp -iv $(KDIR)/System.map /boot/System.map-6.12.0-modded
	cp -iv $(KDIR)/arch/x86/boot/bzImage /boot/vmlinuz-6.12.0-jrathelo-modded
	cp -iv $(KDIR)/.config /boot/config-6.12.0-modded
	grub-mkconfig -o /boot/grub/grub.cfg

install: install_kernel
	reboot

.PHONY: install compile_kernel install_kernel mount_boot restore_kernel
