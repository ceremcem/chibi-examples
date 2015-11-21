# write program to MCU
write: all
	st-flash write build/ch.bin 0x8000000

stop-gdb-server:
	killall st-util 2>/dev/null; true


start-gdb-server: stop-gdb-server
	while true; do st-util; done

# development commands
# --------------------

info:
	@echo
	@echo
	@echo
	@echo "------------------------------------------------------"
	@echo "------------------      DİKKAT    --------------------"
	@echo "başka bir terminalde şunu yap: 'make start-gdb-server'"
	@echo "------------------------------------------------------"
	@echo
	@echo
	@echo
	sleep 1

debug-cmd: all info
	@echo
	@echo
	@echo
	@echo "------------------------------------------------------"
	@echo "------------------      DİKKAT    --------------------"
	@echo "kodu her değiştirdiğinde bu programı tekrar çalıştır "
	@echo "------------------------------------------------------"
	@echo
	@echo
	@echo
	sleep 1
	arm-none-eabi-gdb -x ./gdb-init

#debug-kdbg: all info
#	@echo
#	@echo
#	@echo
#	@echo "------------------------------------------------------"
#	@echo "------------------      DİKKAT    --------------------"
#	@echo " 'debugger executable' kısmında 'arm-none-eabi-gdb' "
#	@echo " belirtildiğinden emin ol!"
#	@echo
#	@echo "     (Settings -> This Program -> Debugger: arm-none-eabi-gdb)"
#	@echo
#	@echo "kodu her değiştirdiğinde bu programı tekrar çalıştır "
#	@echo "------------------------------------------------------"
#	@echo
#	@echo
#	@echo
#	@sleep 2
#	kdbg -r localhost:4242 build/ch.elf

debug-codeblocks: info
	@echo
	@echo
	@echo
	@echo "------------------------------------------------------"
	@echo "------------------      DİKKAT    --------------------"
	@echo " 'debugger executable' kısmında 'arm-none-eabi-gdb' "
	@echo " belirtildiğinden emin ol!"
	@echo "------------------------------------------------------"
	@echo
	@echo
	@echo
	sleep 5

	codeblocks *.cbp > /dev/null

# Code::Blocks specific
Debug: all

cleanDebug: clean

debug-qtcreator: info
	@echo
	@echo
	@echo
	@echo "------------------------------------------------------"
	@echo "------------------      README    --------------------"
	@echo " Qt Creator:"
	@echo "     Debug -> "
	@echo "         Start Debugging -> Attach to remote debug server"
	@echo "         ... (uygun ayarları yap, bkz(README.md))"
	@echo
	@echo "her kodu değiştirdiğinde ayrı bir terminalde şunu yap:"
	@echo
	@echo "    make"
	@echo
	@echo "------------------------------------------------------"
	@echo
	@echo
	@echo
	sleep 5
	qtcreator
