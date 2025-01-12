if ! test -d ./lib/FreeRTOS-Kernel; then
  echo "Cloning FreeRTOS-Kernel"
  git clone https://github.com/FreeRTOS/FreeRTOS-Kernel origim/smp ./lib/FreeRTOS-Kernel
fi

# if ! test -d ./lib/FreeRTOS-Plus-TCP; then
#   echo "Cloning FreeRTOS-Plus-TCP"
#   git clone https://github.com/FreeRTOS/FreeRTOS-Plus-TCP ./lib/FreeRTOS-Plus-TCP
# fi
