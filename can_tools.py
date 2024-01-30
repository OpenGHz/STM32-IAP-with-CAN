#!/usr/bin/env python

import can
from typing import Optional, List
import time
from multiprocessing import Queue
from threading import Thread

"""
    https://python-can.readthedocs.io/en/stable/
    The can object exposes an rc dictionary which can be used to set the interface and channel.

    import can
    can.rc['interface'] = 'socketcan'
    can.rc['channel'] = 'vcan0'
    can.rc['bitrate'] = 500000
    from can.interface import Bus

    bus = Bus()
    You can also specify the interface and channel for each Bus instance:

    import can

    bus = can.interface.Bus(interface='socketcan', channel='vcan0', bitrate=500000)
"""


class CanTool:
    def __init__(
        self,
        device: Optional[str],
        channel: Optional[str],
        bitrate: Optional[int],
        maxsize: int = 0,
    ):
        """
        功能：初始化类时即创建总线对象
        param1：设备名，如：socketcan, pcan, ixxat, vector
        param2：通道名，如：vcan0, PCAN_USBBUS1, 0, 0
        param3：波特率，如：250000, 500000, 1000000
        param4：队列大小
        """
        self._mq = Queue(maxsize)  # 创建一个有限队列
        self._last_msg = None  # 用于存储最后一次接收到的报文
        self._bus = can.interface.Bus(
            bustype=device, channel=channel, bitrate=bitrate
        )  # 是否在初始化时创建对象基于项目而定
        logger = can.Logger("logfile.asc")  # 创建一个log文件
        listeners = [
            self.__put_mq,  # 回调函数
            logger,  # 保存报文的对象
        ]
        self.notifier = can.Notifier(self._bus, listeners)  # 设置一个监听
        self.target_filter_id_set = set()  # 用于存储需要筛选的id

    def __put_mq(self, msg):
        """
        功能：通过队列实现数据的写入和读取
        :param msg: 回调返回的对象
        """
        self._last_msg = msg
        if not self._mq.full():
            self._mq.put_nowait(msg)
        else:
            print("queue is full")
            self._mq.get_nowait()
            self._mq.put_nowait(msg)

    def send_data(
        self,
        id: Optional[int],
        listdata: Optional[List[int]],
        is_extended_id: Optional[bool] = False,
    ):
        """
        功能：发送数据
        para1：报文id
        para2：报文数据
        para3：是否为扩展帧
        """
        msg = can.Message(
            arbitration_id=id,
            is_extended_id=is_extended_id,
            data=listdata,
        )
        self._bus.send(msg)

    def set_filters(self, filter_id: Optional[int]):
        """
        功能：设置一个筛选器
        para1：报文id
        """
        if filter_id not in self.target_filter_id_set:
            self.target_filter_id_set.add(filter_id)
            if filter_id < 2147483648:  # 判断是否为标准帧
                self._bus.set_filters(
                    [{"can_id": filter_id, "can_mask": 0x7FF, "extended": False}]
                )
            else:
                self._bus.set_filters(
                    [{"can_id": filter_id, "can_mask": 0x1FFFFFFF, "extended": True}]
                )
        else:
            print("filter already exists")

    def can_recv(self, filter_id: Optional[int]):
        """
        循环检测接收到的报文中是否有需要的id
        para1：报文id
        """
        if filter_id not in self.target_filter_id_set:
            self.target_filter_id_set.add(filter_id)
            self.set_filters(filter_id)

            def can_recv_thread(freq: int = 10):
                while True:
                    if not self._mq.empty():
                        frame: can.Message = self._mq.get_nowait()
                        if filter_id == frame.arbitration_id:
                            print(frame.data)
                            break
                    time.sleep(1 / freq)

            Thread(target=can_recv_thread, daemon=True).start()


"""
This example shows how sending a single message works.
"""


def send_one():
    """Sends a single message."""

    # this uses the default configuration (for example from the config file)
    # see https://python-can.readthedocs.io/en/stable/configuration.html
    with can.Bus() as bus:
        # Using specific buses works similar:
        # bus = can.Bus(interface='socketcan', channel='vcan0', bitrate=250000)
        # bus = can.Bus(interface='pcan', channel='PCAN_USBBUS1', bitrate=250000)
        # bus = can.Bus(interface='ixxat', channel=0, bitrate=250000)
        # bus = can.Bus(interface='vector', app_name='CANalyzer', channel=0, bitrate=250000)
        # ...

        msg = can.Message(
            arbitration_id=0xC0FFEE, data=[0, 25, 0, 1, 3, 1, 4, 1], is_extended_id=True
        )

        try:
            bus.send(msg)
            print(f"Message sent on {bus.channel_info}")
        except can.CanError:
            print("Message NOT sent")


if __name__ == "__main__":
    send_one()
