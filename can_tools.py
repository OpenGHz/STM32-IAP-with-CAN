#!/usr/bin/env python

import can
from typing import Optional, List, Union
import time
from multiprocessing import Queue
from threading import Thread
from bin_tools import BinTools

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
    # Using specific buses works similar:
    bus = can.interface.Bus(interface='socketcan', channel='vcan0', bitrate=500000)
    # bus = can.Bus(interface='socketcan', channel='vcan0', bitrate=250000)
    # bus = can.Bus(interface='pcan', channel='PCAN_USBBUS1', bitrate=250000)
    # bus = can.Bus(interface='ixxat', channel=0, bitrate=250000)
    # bus = can.Bus(interface='vector', app_name='CANalyzer', channel=0, bitrate=250000)

    # CanData: typing.Union[bytes, bytearray, int, typing.Iterable[int]]
    data = [0, 25, 0, 1, 3, 1, 4, 1]
    # data = b'\x00\x19\x00\x01\x03\x01\x04\x01'

"""


class Caner:
    def __init__(
        self,
        channel: Optional[str] = 0,
        interface: Optional[str] = "socketcan",
        bitrate: Optional[int] = 1e6,
        listen_size: Optional[int] = 1e3,
    ):
        """
        功能：初始化类时即创建总线对象
        param1：通道名，如：vcan0, PCAN_USBBUS1, 0
        param2：接口类型，如：socketcan, pcan, ixxat, vector
        param3：波特率，如：250000, 500000, 1000000
        param4：队列大小，0为无限制，None为不listen

        数据发送不需要指定data的长度，会自动计算
        """
        self._last_msg = None  # 用于存储最后一次接收到的报文
        self._bus = can.interface.Bus(channel, interface, bitrate=bitrate)
        self.target_filter_id_set = set()  # 用于存储需要筛选的id

        self.listen_size = listen_size
        if listen_size is not None:
            self._msg_queue = Queue(listen_size)  # 创建一个有限队列
            logger = can.Logger("logfile.asc")  # 创建一个log文件
            listeners = [
                self.__put_msg,  # 回调函数
                logger,  # 保存报文的对象
            ]
            self.notifier = can.Notifier(self._bus, listeners)  # 设置一个监听

    def __put_msg(self, msg):
        """
        功能：通过队列实现数据的写入和读取
        :param msg: 回调返回的对象
        """
        self._last_msg = msg
        if not self._msg_queue.full():
            self._msg_queue.put_nowait(msg)
        else:
            print("queue is full")
            self._msg_queue.get_nowait()
            self._msg_queue.put_nowait(msg)

    def clear_received_msg(self):
        """
        功能：清空接收到的报文
        """
        self._msg_queue = Queue(self.listen_size)

    @staticmethod
    def create_basic_msg(
        arbitration_id: Optional[int],
        data: Optional[List[int]],
        is_extended_id: Optional[bool] = False,
    ) -> can.Message:
        """
        功能：创建一个基本报文对象
        para1：报文id
        para2：是否为扩展帧
        """
        return can.Message(
            arbitration_id=arbitration_id,
            is_extended_id=is_extended_id,
            data=data,
        )

    def send_msg(self, msg: can.Message):
        """
        功能：发送报文
        para1：报文对象
        """
        self._bus.send(msg)

    def send_data(
        self,
        arbitration_id: Optional[int],
        data: Union[can.typechecking.CanData, str],
        is_extended_id: Optional[bool] = False,
    ):
        """
        功能：发送数据
        para1：报文id
        para2：报文数据，如：'0011223344556677'，不能直接发送0x0011223344556677，前面两个00会被忽略
        para3：是否为扩展帧
        """
        if isinstance(data, str):
            data = bytes.fromhex(data)
        msg = can.Message(
            arbitration_id=arbitration_id,
            is_extended_id=is_extended_id,
            data=data,
        )
        self._bus.send(msg)

    def send_str_hex(self, arbitration_id: Optional[int], data: str):
        """
        功能：发送16进制字符串
        para1：报文id
        para2：报文数据，如：'0011223344556677'，无Ox前缀
        """
        # 数据准备
        self.send_data(arbitration_id, bytes.fromhex(data))

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
                    if not self._msg_queue.empty():
                        frame: can.Message = self._msg_queue.get_nowait()
                        if filter_id == frame.arbitration_id:
                            print(frame.data)
                            break
                    time.sleep(1 / freq)

            Thread(target=can_recv_thread, daemon=True).start()

    def send_file(
        id: Optional[int],
        file_path: str,
        bytes_per: Optional[int] = 8,
        freq=100,
        is_extended_id: Optional[bool] = False,
    ):
        """
        功能：发送文件到CAN
        :param id: 报文id
        :param file_path: 文件路径
        :param is_extended_id: 是否为扩展帧
        :param bytes_per: 每次发送的字节数，范围为[1, 8], 默认为一次发送8字节
        :param freq: 发送频率，单位为Hz
        """
        if not BinTools.check_file(file_path):
            raise Exception("file not exists")
        if bytes_per <= 0 or bytes_per > 8:
            raise Exception("bytes_per must be in [1, 8]")
        data = BinTools.read_bin_file(file_path)
        data_len = len(data)
        period = 1 / freq
        for i in range(0, data_len, bytes_per):
            Caner.send_data(id, data[i : i + bytes_per], is_extended_id)
            time.sleep(period)


if __name__ == "__main__":
    """
    This example shows how sending a single message works.
    """

    def send_one():
        """Sends a single message."""
        # this uses the default configuration (for example from the config file)
        # see https://python-can.readthedocs.io/en/stable/configuration.html
        with can.Bus() as bus:
            msg = can.Message(
                arbitration_id=0xC0FFEE,
                data=[0, 25, 0, 1, 3, 1, 4, 1],
                is_extended_id=True,
            )

            try:
                bus.send(msg)
                print(f"Message sent on {bus.channel_info}")
            except can.CanError:
                print("Message NOT sent")

    send_one()
