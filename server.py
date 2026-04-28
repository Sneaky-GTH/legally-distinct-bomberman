# Simple server implementation for this game. Does not do anything except accept connections and send valid messages to the client.

from typing import Literal
import socket
import threading
import traceback

def uint8_en(value) -> bytes:
    return value.to_bytes(1, byteorder='big')

def uint8_de(value) -> int:
    return int.from_bytes(value, byteorder='big')

def uint16_en(value) -> bytes:
    return value.to_bytes(2, byteorder='big')

def uint16_de(value) -> int:
    return int.from_bytes(value, byteorder='big')

def cstr_en(value: str, pad: int | None = None) -> bytes:
    encoded = value.encode('utf-8')
    encoded += b'\x00' # Null terminator
    if pad is not None:
        if len(encoded) < pad:
            encoded += b'\x00' * (pad - len(encoded))
        if len(encoded) > pad:
            print(f"Warning: String '{value}' is too long to fit in {pad} bytes, truncating")
            encoded = encoded[:pad - 1] + b'\x00' # Ensure null terminator is still present
    return encoded

def cstr_de(value: bytes) -> str:
    return value.split(b'\x00', 1)[0].decode('utf-8')

class NotEnoughDataError(Exception):
    pass

class MessageHandleError(Exception):
    pass

class Message:
    # uint8 type
    # uint8 sender_id
    # uint8 target_id (255 for server, 254 for broadcast)
    # ... rest of the message depends on the type
    
    def __init__(self, sender_id: int, target_id: int):
        self.sender_id = sender_id
        self.target_id = target_id

    def __str__(self):
        return f"{self.__class__.__name__}(sender_id={self.sender_id}, target_id={self.target_id})"

    @property
    def type(self) -> int:
        traceback.print_stack()
        raise NotImplementedError("Subclasses must implement type property")

    def encode(self) -> bytes:
        traceback.print_stack()
        raise NotImplementedError("Subclasses must implement encode() method")

    @staticmethod
    def decode(data: bytes) -> tuple["Message", int]:
        traceback.print_stack()
        raise NotImplementedError("Subclasses must implement decode() method")

    @staticmethod
    def base_decode(data: bytes) -> tuple[int, int, int]:
        if len(data) < 3:
            raise NotEnoughDataError("Not enough data to decode base message")
        message_type = uint8_de(data[0:1])
        sender = uint8_de(data[1:2])
        target = uint8_de(data[2:3])
        return message_type, sender, target

    def base_encode(self) -> bytes:
        return uint8_en(self.type) + uint8_en(self.sender_id) + uint8_en(self.target_id)

class DegenerateMessage(Message):
    # ...
    # No additional data

    @staticmethod
    def decode(data: bytes) -> tuple["DegenerateMessage", int]:
        if len(data) < 3:
            raise NotEnoughDataError("Not enough data to decode DegenerateMessage")
        message_type, sender, target = Message.base_decode(data)
        return DegenerateMessage(sender, target), 3

    def encode(self) -> bytes:
        return self.base_encode()

class HelloMessage(Message):
    # ...
    # 20-byte null-padded string for client ID,
    # 30-byte null-padded string for client name
    def __init__(self, sender_id: int, target_id: int, id: str, name: str):
        super().__init__(sender_id=sender_id, target_id=target_id)
        self.id = id
        self.name = name

    def __str__(self):
        return f"{self.__class__.__name__}(sender_id={self.sender_id}, target_id={self.target_id}, id='{self.id}', name='{self.name}')"

    @property
    def type(self) -> int:
        return 0

    @staticmethod
    def decode(data: bytes) -> tuple["HelloMessage", int]:
        message_type, sender, target = Message.base_decode(data)
        if message_type != 0:
            raise ValueError(f"Invalid message type for HelloMessage: {message_type}")
        id = cstr_de(data[3:23])
        name = cstr_de(data[23:53])
        return HelloMessage(sender, target, id, name), 53 # Always 53 bytes for HelloMessage

    def encode(self) -> bytes:
        return self.base_encode() + cstr_en(self.id, pad=20) + cstr_en(self.name, pad=30)


class WelcomeMessage(Message):
    # ...
    # 20-byte null-padded string server ID,
    # uint8 status (0 for waiting, 1 for in-game, 2 for game ended),
    # uint8 client count,
    # For each client:
    #   uint8 client ID,
    #   uint8 ready status (0 for not ready, 1 for ready),
    #   30-byte null-padded string client name
    def __init__(self, sender_id: int, target_id: int, server_id: str, status: int, clients: list[tuple[int, bool, str]]):
        super().__init__(sender_id=sender_id, target_id=target_id)
        self.server_id = server_id
        self.status = status
        self.clients = clients

    def __str__(self):
        return f"{self.__class__.__name__}(sender_id={self.sender_id}, target_id={self.target_id}, server_id='{self.server_id}', status={self.status}, clients={self.clients})"

    @property
    def type(self) -> int:
        return 1

    @staticmethod
    def decode(data: bytes) -> tuple["WelcomeMessage", int]:
        message_type, sender, target = Message.base_decode(data)
        if message_type != 1:
            raise ValueError(f"Invalid message type for WelcomeMessage: {message_type}")
        server_id = cstr_de(data[3:23])
        status = uint8_de(data[23:24])
        clients = list[tuple[int, bool, str]]()  # Placeholder for client list parsing
        client_count = uint8_de(data[24:25])
        current_byte = 25
        for _ in range(client_count):
            if current_byte + 21 > len(data):
                raise NotEnoughDataError("Not enough data for client information")
            client_id = uint8_de(data[current_byte:current_byte+1])
            current_byte += 1
            if current_byte + 1 > len(data):
                raise NotEnoughDataError("Not enough data for client ready status")
            ready_status = uint8_de(data[current_byte:current_byte+1])
            if ready_status not in (0, 1):
                raise ValueError(f"Invalid ready status for client: {ready_status}")
            current_byte += 1
            client_name = cstr_de(data[current_byte:current_byte+30])
            current_byte += 30
            clients.append((client_id, bool(ready_status), client_name))
        return WelcomeMessage(sender, target, server_id, status, clients), current_byte

    def encode(self) -> bytes:
        data = self.base_encode() + cstr_en(self.server_id, pad=20) + uint8_en(self.status) + uint8_en(len(self.clients))
        for client_id, ready_status, client_name in self.clients:
            data += uint8_en(client_id) + uint8_en(ready_status) + cstr_en(client_name, pad=30)
        return data


class LeaveMessage(DegenerateMessage):
    @property
    def type(self) -> int:
        return 5

class DisconnectMessage(DegenerateMessage):
    @property
    def type(self) -> int:
        return 2

class PingMessage(DegenerateMessage):
    @property
    def type(self) -> int:
        return 3

class PongMessage(DegenerateMessage):
    @property
    def type(self) -> int:
        return 4

class ErrorMessage(Message):
    # ...
    # 50-byte null-padded string error message
    def __init__(self, sender_id: int, target_id: int, error_message: str):
        super().__init__(sender_id=sender_id, target_id=target_id)
        self.error_message = error_message

    def __str__(self):
        return f"{self.__class__.__name__}(sender_id={self.sender_id}, target_id={self.target_id}, error_message='{self.error_message}')"

    @property
    def type(self) -> int:
        return 6

    @staticmethod
    def decode(data: bytes) -> tuple["ErrorMessage", int]:
        message_type, sender, target = Message.base_decode(data)
        if message_type != 6:
            raise ValueError(f"Invalid message type for ErrorMessage: {message_type}")
        error_message = cstr_de(data[3:53])
        return ErrorMessage(sender, target, error_message), 53

    def encode(self) -> bytes:
        return self.base_encode() + cstr_en(self.error_message, pad=50)

class SetReadyMessage(DegenerateMessage):
    @property
    def type(self) -> int:
        return 10

class SetStatusMessage(Message):
    # ...
    # uint8 new status (0 for waiting, 1 for in-game, 2 for game ended)
    def __init__(self, sender_id: int, target_id: int, new_status: int):
        super().__init__(sender_id=sender_id, target_id=target_id)
        self.new_status = new_status

    def __str__(self):
        return f"{self.__class__.__name__}(sender_id={self.sender_id}, target_id={self.target_id}, new_status={self.new_status})"

    @property
    def type(self) -> int:
        return 20

    @staticmethod
    def decode(data: bytes) -> tuple["SetStatusMessage", int]:
        message_type, sender, target = Message.base_decode(data)
        if message_type != 20:
            raise ValueError(f"Invalid message type for SetStatusMessage: {message_type}")
        new_status = uint8_de(data[3:4])
        return SetStatusMessage(sender, target, new_status), 4

    def encode(self) -> bytes:
        return self.base_encode() + uint8_en(self.new_status)

class WinnerMessage(Message):
    # ...
    # uint8 winner client ID
    def __init__(self, sender_id: int, target_id: int, winner_id: int):
        super().__init__(sender_id=sender_id, target_id=target_id)
        self.winner_id = winner_id

    def __str__(self):
        return f"{self.__class__.__name__}(sender_id={self.sender_id}, target_id={self.target_id}, winner_id={self.winner_id})"

    @property
    def type(self) -> int:
        return 23

    @staticmethod
    def decode(data: bytes) -> tuple["WinnerMessage", int]:
        message_type, sender, target = Message.base_decode(data)
        if message_type != 23:
            raise ValueError(f"Invalid message type for WinnerMessage: {message_type}")
        winner_id = uint8_de(data[3:4])
        return WinnerMessage(sender, target, winner_id), 4

    def encode(self) -> bytes:
        return self.base_encode() + uint8_en(self.winner_id)

class MapMessage(Message):
    # ...
    # uint8 width,
    # uint8 height,
    # For each cell:
    #   uint8 cell type:
    #    . for empty,
    #    H for wall,
    #    S for breakable wall,
    #    B for bomb,
    #    1..8 for player spawn,
    #   A, R, T for bonus
    def __init__(self, sender_id: int, target_id: int, width: int, height: int, cells: list[int]):
        super().__init__(sender_id=sender_id, target_id=target_id)
        self.width = width
        self.height = height
        self.cells = cells

    def __str__(self):
        return f"{self.__class__.__name__}(sender_id={self.sender_id}, target_id={self.target_id}, width={self.width}, height={self.height}, cells={self.cells})"

    @property
    def type(self) -> int:
        return 7

    @staticmethod
    def decode(data: bytes) -> tuple["MapMessage", int]:
        message_type, sender, target = Message.base_decode(data)
        if message_type != 7:
            raise ValueError(f"Invalid message type for MapMessage: {message_type}")
        if len(data) < 5:
            raise NotEnoughDataError("Not enough data to decode MapMessage header")
        width = uint8_de(data[3:4])
        height = uint8_de(data[4:5])
        expected_length = 5 + width * height
        if len(data) < expected_length:
            raise NotEnoughDataError("Not enough data to decode MapMessage cells")
        cells = list[int]()
        for i in range(width * height):
            cell_type = uint8_de(data[5+i:6+i])
            cells.append(cell_type)
        return MapMessage(sender, target, width, height, cells), expected_length

    def encode(self) -> bytes:
        data = self.base_encode() + uint8_en(self.width) + uint8_en(self.height)
        for cell in self.cells:
            data += uint8_en(cell)
        return data

class MoveAttemptMessage(Message):
    # ...
    # uint8 direction (U for up, R for right, D for down, L for left)
    def __init__(self, sender_id: int, target_id: int, direction: int):
        super().__init__(sender_id=sender_id, target_id=target_id)
        self.direction = direction

    def __str__(self):
        return f"{self.__class__.__name__}(sender_id={self.sender_id}, target_id={self.target_id}, direction={self.direction})"

    @property
    def type(self) -> int:
        return 30

    @staticmethod
    def decode(data: bytes) -> tuple["MoveAttemptMessage", int]:
        message_type, sender, target = Message.base_decode(data)
        if message_type != 30:
            raise ValueError(f"Invalid message type for MoveAttemptMessage: {message_type}")
        if len(data) < 4:
            raise NotEnoughDataError("Not enough data to decode MoveAttemptMessage")
        direction = uint8_de(data[3:4])
        return MoveAttemptMessage(sender, target, direction), 4

    def encode(self) -> bytes:
        return self.base_encode() + uint8_en(self.direction)

class MovedMessage(Message):
    # ...
    # uint8 player,
    # uint16 new position (packed as row * cols + col),
    def __init__(self, sender_id: int, target_id: int, player: int, position: int):
        super().__init__(sender_id=sender_id, target_id=target_id)
        self.player = player
        self.position = position

    def __str__(self):
        return f"{self.__class__.__name__}(sender_id={self.sender_id}, target_id={self.target_id}, player={self.player}, position={self.position})"

    @property
    def type(self) -> int:
        return 40

    @staticmethod
    def decode(data: bytes) -> tuple["MovedMessage", int]:
        message_type, sender, target = Message.base_decode(data)
        if message_type != 40:
            raise ValueError(f"Invalid message type for MovedMessage: {message_type}")
        if len(data) < 5:
            raise NotEnoughDataError("Not enough data to decode MovedMessage")

        player = uint8_de(data[3:4])
        position = uint16_de(data[4:6])
        return MovedMessage(sender, target, player, position), 6

    def encode(self) -> bytes:
        return self.base_encode() + uint16_en(self.position)

class BombAttemptMessage(Message):
    # ...
    # uint16 for position (packed as row * cols + col)
    def __init__(self, sender_id: int, target_id: int, position: int):
        super().__init__(sender_id=sender_id, target_id=target_id)
        self.position = position

    def __str__(self):
        return f"{self.__class__.__name__}(sender_id={self.sender_id}, target_id={self.target_id}, position={self.position})"

    @property
    def type(self) -> int:
        return 31

    @staticmethod
    def decode(data: bytes) -> tuple["BombAttemptMessage", int]:
        message_type, sender, target = Message.base_decode(data)
        if message_type != 31:
            raise ValueError(f"Invalid message type for BombAttemptMessage: {message_type}")
        if len(data) < 5:
            raise NotEnoughDataError("Not enough data to decode BombAttemptMessage")
        position = uint16_de(data[3:5])
        return BombAttemptMessage(sender, target, position), 5

    def encode(self) -> bytes:
        return self.base_encode() + uint16_en(self.position)

class BombMessage(Message):
    # ...
    # uint8 player,
    # uint16 for position (packed as row * cols + col)
    def __init__(self, sender_id: int, target_id: int, player: int, position: int):
        super().__init__(sender_id=sender_id, target_id=target_id)
        self.player = player
        self.position = position

    def __str__(self):
        return f"{self.__class__.__name__}(sender_id={self.sender_id}, target_id={self.target_id}, player={self.player}, position={self.position})"

    @property
    def type(self) -> int:
        return 41

    @staticmethod
    def decode(data: bytes) -> tuple["BombMessage", int]:
        message_type, sender, target = Message.base_decode(data)
        if message_type != 41:
            raise ValueError(f"Invalid message type for BombMessage: {message_type}")
        if len(data) < 5:
            raise NotEnoughDataError("Not enough data to decode BombMessage")
        player = uint8_de(data[3:4])
        position = uint16_de(data[4:6])
        return BombMessage(sender, target, player, position), 6

    def encode(self) -> bytes:
        return self.base_encode() + uint8_en(self.player) + uint16_en(self.position)

class ExplosionStartMessage(Message):
    # ...
    # uint8 radius,
    # uint16 for position (packed as row * cols + col)
    def __init__(self, sender_id: int, target_id: int, radius: int, position: int):
        super().__init__(sender_id=sender_id, target_id=target_id)
        self.radius = radius
        self.position = position

    def __str__(self):
        return f"{self.__class__.__name__}(sender_id={self.sender_id}, target_id={self.target_id}, radius={self.radius}, position={self.position})"

    @property
    def type(self) -> int:
        return 42

    @staticmethod
    def decode(data: bytes) -> tuple["ExplosionStartMessage", int]:
        message_type, sender, target = Message.base_decode(data)
        if message_type != 42:
            raise ValueError(f"Invalid message type for ExplosionStartMessage: {message_type}")
        if len(data) < 6:
            raise NotEnoughDataError("Not enough data to decode ExplosionStartMessage")
        radius = uint8_de(data[3:4])
        position = uint16_de(data[4:6])
        return ExplosionStartMessage(sender, target, radius, position), 6

    def encode(self) -> bytes:
        return self.base_encode() + uint8_en(self.radius) + uint16_en(self.position)

class ExplosionEndMessage(Message):
    # ...
    # uint8 radius,
    # uint16 for position (packed as row * cols + col)
    def __init__(self, sender_id: int, target_id: int, radius: int, position: int):
        super().__init__(sender_id=sender_id, target_id=target_id)
        self.radius = radius
        self.position = position

    def __str__(self):
        return f"{self.__class__.__name__}(sender_id={self.sender_id}, target_id={self.target_id}, radius={self.radius}, position={self.position})"

    @property
    def type(self) -> int:
        return 43

    @staticmethod
    def decode(data: bytes) -> tuple["ExplosionEndMessage", int]:
        message_type, sender, target = Message.base_decode(data)
        if message_type != 43:
            raise ValueError(f"Invalid message type for ExplosionEndMessage: {message_type}")
        if len(data) < 6:
            raise NotEnoughDataError("Not enough data to decode ExplosionEndMessage")
        radius = uint8_de(data[3:4])
        position = uint16_de(data[4:6])
        return ExplosionEndMessage(sender, target, radius, position), 6

    def encode(self) -> bytes:
        return self.base_encode() + uint8_en(self.radius) + uint16_en(self.position)

class DeathMessage(Message):
    # ...
    # uint8 player
    def __init__(self, sender_id: int, target_id: int, player: int):
        super().__init__(sender_id=sender_id, target_id=target_id)
        self.player = player

    def __str__(self):
        return f"{self.__class__.__name__}(sender_id={self.sender_id}, target_id={self.target_id}, player={self.player})"

    @property
    def type(self) -> int:
        return 44

    @staticmethod
    def decode(data: bytes) -> tuple["DeathMessage", int]:
        message_type, sender, target = Message.base_decode(data)
        if message_type != 44:
            raise ValueError(f"Invalid message type for DeathMessage: {message_type}")
        if len(data) < 4:
            raise NotEnoughDataError("Not enough data to decode DeathMessage")
        player = uint8_de(data[3:4])
        return DeathMessage(sender, target, player), 4

class BonusAvailableMessage(Message):
    # ...
    # uint8 bonus type (A, R, or T)
    # uint16 for position (packed as row * cols + col)
    def __init__(self, sender_id: int, target_id: int, bonus_type: int, position: int):
        super().__init__(sender_id=sender_id, target_id=target_id)
        self.bonus_type = bonus_type
        self.position = position

    def __str__(self):
        return f"{self.__class__.__name__}(sender_id={self.sender_id}, target_id={self.target_id}, bonus_type={self.bonus_type}, position={self.position})"

    @property
    def type(self) -> int:
        return 45

    @staticmethod
    def decode(data: bytes) -> tuple["BonusAvailableMessage", int]:
        message_type, sender, target = Message.base_decode(data)
        if message_type != 45:
            raise ValueError(f"Invalid message type for BonusAvailableMessage: {message_type}")
        if len(data) < 6:
            raise NotEnoughDataError("Not enough data to decode BonusAvailableMessage")
        bonus_type = uint8_de(data[3:4])
        if bonus_type not in (ord('A'), ord('R'), ord('T')):
            raise ValueError(f"Invalid bonus type for BonusAvailableMessage: {bonus_type}")
        position = uint16_de(data[4:6])
        return BonusAvailableMessage(sender, target, bonus_type, position), 6

class BonusRetrievedMessage(Message):
    # ...
    # uint8 player,
    # uint16 for position (packed as row * cols + col)
    def __init__(self, sender_id: int, target_id: int, player: int, position: int):
        super().__init__(sender_id=sender_id, target_id=target_id)
        self.player = player
        self.position = position

    def __str__(self):
        return f"{self.__class__.__name__}(sender_id={self.sender_id}, target_id={self.target_id}, player={self.player}, position={self.position})"

    @property
    def type(self) -> int:
        return 46

    @staticmethod
    def decode(data: bytes) -> tuple["BonusRetrievedMessage", int]:
        message_type, sender, target = Message.base_decode(data)
        if message_type != 46:
            raise ValueError(f"Invalid message type for BonusRetrievedMessage: {message_type}")
        if len(data) < 6:
            raise NotEnoughDataError("Not enough data to decode BonusRetrievedMessage")
        player = uint8_de(data[3:4])
        position = uint16_de(data[4:6])
        return BonusRetrievedMessage(sender, target, player, position), 6

    def encode(self) -> bytes:
        return self.base_encode() + uint8_en(self.player) + uint16_en(self.position)

class BlockDestroyedMessage(Message):
    # ...
    # uint16 for position (packed as row * cols + col)
    def __init__(self, sender_id: int, target_id: int, position: int):
        super().__init__(sender_id=sender_id, target_id=target_id)
        self.position = position

    def __str__(self):
        return f"{self.__class__.__name__}(sender_id={self.sender_id}, target_id={self.target_id}, position={self.position})"

    @property
    def type(self) -> int:
        return 47

    @staticmethod
    def decode(data: bytes) -> tuple["BlockDestroyedMessage", int]:
        message_type, sender, target = Message.base_decode(data)
        if message_type != 47:
            raise ValueError(f"Invalid message type for BlockDestroyedMessage: {message_type}")
        if len(data) < 5:
            raise NotEnoughDataError("Not enough data to decode BlockDestroyedMessage")
        position = uint16_de(data[3:5])
        return BlockDestroyedMessage(sender, target, position), 5

    def encode(self) -> bytes:
        return self.base_encode() + uint16_en(self.position)

def parse_message(data: bytes) -> tuple[Message, bytes, Literal[False]] | tuple[None, bytes, Literal[True]]:
    if len(data) < 1:
        return None, b"", True # Not enough data to determine message type

    message_type = uint8_de(data[0:1])
    print(f"Received message type: {message_type}")

    try:
        match message_type:
            case 0: # MSG_HELLO
                msg, consumed_bytes = HelloMessage.decode(data)
                return msg, data[consumed_bytes:], False
            case 1: # MSG_WELCOME
                msg, consumed_bytes = WelcomeMessage.decode(data)
                return msg, data[consumed_bytes:], False
            case 2: # MSG_DISCONNECT
                msg, consumed_bytes = DisconnectMessage.decode(data)
                return msg, data[consumed_bytes:], False
            case 3: # MSG_PING
                msg, consumed_bytes = PingMessage.decode(data)
                return msg, data[consumed_bytes:], False
            case 4: # MSG_PONG
                msg, consumed_bytes = PongMessage.decode(data)
                return msg, data[consumed_bytes:], False
            case 5: # MSG_LEAVE
                msg, consumed_bytes = LeaveMessage.decode(data)
                return msg, data[consumed_bytes:], False
            case 6: # MSG_ERROR
                msg, consumed_bytes = ErrorMessage.decode(data)
                return msg, data[consumed_bytes:], False
            case 10: # MSG_SET_READY
                msg, consumed_bytes = SetReadyMessage.decode(data)
                return msg, data[consumed_bytes:], False
            case 20: # MSG_SET_STATUS
                msg, consumed_bytes = SetStatusMessage.decode(data)
                return msg, data[consumed_bytes:], False
            case 23: # MSG_WINNER
                msg, consumed_bytes = WinnerMessage.decode(data)
                return msg, data[consumed_bytes:], False
            case 30: # MSG_MOVE_ATTEMPT
                msg, consumed_bytes = MoveAttemptMessage.decode(data)
                return msg, data[consumed_bytes:], False
            case 31: # MSG_BOMB_ATTEMPT
                msg, consumed_bytes = BombAttemptMessage.decode(data)
                return msg, data[consumed_bytes:], False
            case 40: # MSG_MOVED
                msg, consumed_bytes = MovedMessage.decode(data)
                return msg, data[consumed_bytes:], False
            case 41: # MSG_BOMB
                msg, consumed_bytes = BombMessage.decode(data)
                return msg, data[consumed_bytes:], False
            case 42: # MSG_EXPLOSION_START
                msg, consumed_bytes = ExplosionStartMessage.decode(data)
                return msg, data[consumed_bytes:], False
            case 43: # MSG_EXPLOSION_END
                msg, consumed_bytes = ExplosionEndMessage.decode(data)
                return msg, data[consumed_bytes:], False
            case 44: # MSG_DEATH
                msg, consumed_bytes = DeathMessage.decode(data)
                return msg, data[consumed_bytes:], False
            case 45: # MSG_BONUS_AVAILABLE
                msg, consumed_bytes = BonusAvailableMessage.decode(data)
                return msg, data[consumed_bytes:], False
            case 46: # MSG_BONUS_RETRIEVED
                msg, consumed_bytes = BonusRetrievedMessage.decode(data)
                return msg, data[consumed_bytes:], False
            case 47: # MSG_BLOCK_DESTROYED
                msg, consumed_bytes = BlockDestroyedMessage.decode(data)
                return msg, data[consumed_bytes:], False
            # case 100: # MSG_SYNC_BOARD
            # case 101: # MSG_SYNC_REQUEST
            case _:
                raise ValueError(f"Unknown message type: {message_type}")
    except NotEnoughDataError:
        return None, data, True # Not enough data to decode the message, wait for more

def send_message(client_socket: socket.socket, message: Message):
    print(f"Sending message: {message}")
    client_socket.sendall(message.encode())

def handle_message(client_socket: socket.socket, message: Message):
    print(f"Handling message: {message}")

    if isinstance(message, HelloMessage):
        print(f"Received HelloMessage from client {message.sender_id} with ID '{message.id}' and name '{message.name}'")
        welcome_msg = WelcomeMessage(sender_id=255, target_id=message.sender_id, server_id="Test Server", status=0, clients=[(message.sender_id, False, message.name)])
        send_message(client_socket, welcome_msg)
        return

    if isinstance(message, WelcomeMessage):
        print(f"Received WelcomeMessage from client {message.sender_id} with server ID '{message.server_id}' and status {message.status}")
        raise MessageHandleError("WELCOME can only be sent by the server")

    if isinstance(message, DisconnectMessage):
        print(f"Received DisconnectMessage from client {message.sender_id}")
        raise MessageHandleError("DISCONNECT can only be sent by the server")

    if isinstance(message, PingMessage):
        print(f"Received PingMessage from client {message.sender_id}, sending PongMessage in response")
        pong_msg = PongMessage(sender_id=255, target_id=message.sender_id)
        send_message(client_socket, pong_msg)
        return

    if isinstance(message, PongMessage):
        print(f"Received PongMessage from client {message.sender_id}")
        return # No response needed for PongMessage

    if isinstance(message, ErrorMessage):
        print(f"Received ErrorMessage from client {message.sender_id} with error message: '{message.error_message}'")
        raise MessageHandleError("ERROR can only be sent by the server")

    if isinstance(message, SetReadyMessage):
        print(f"Received SetReadyMessage from client {message.sender_id}")
        return # TODO

    if isinstance(message, SetStatusMessage):
        print(f"Received SetStatusMessage from client {message.sender_id} with new status {message.new_status}")
        return # TODO

    if isinstance(message, WinnerMessage):
        print(f"Received WinnerMessage from client {message.sender_id} with winner ID {message.winner_id}")
        raise MessageHandleError("WINNER can only be sent by the server")

    if isinstance(message, MapMessage):
        print(f"Received MapMessage from client {message.sender_id} with width {message.width} and height {message.height}")
        raise MessageHandleError("MAP can only be sent by the server")

    if isinstance(message, MoveAttemptMessage):
        print(f"Received MoveAttemptMessage from client {message.sender_id} with direction {message.direction}")
        return # TODO

    if isinstance(message, MovedMessage):
        print(f"Received MovedMessage from client {message.sender_id} for player {message.player} to position {message.position}")
        raise MessageHandleError("MOVED can only be sent by the server")

    if isinstance(message, BombAttemptMessage):
        print(f"Received BombAttemptMessage from client {message.sender_id} with position ({message.position})")
        return # TODO

    if isinstance(message, BombMessage):
        print(f"Received BombMessage from client {message.sender_id} for player {message.player} at position {message.position}")
        raise MessageHandleError("BOMB can only be sent by the server")

    if isinstance(message, ExplosionStartMessage):
        print(f"Received ExplosionStartMessage from client {message.sender_id} with radius {message.radius} at position ({message.position})")
        raise MessageHandleError("EXPLOSION_START can only be sent by the server")

    if isinstance(message, ExplosionEndMessage):
        print(f"Received ExplosionEndMessage from client {message.sender_id} with radius {message.radius} at position ({message.position})")
        raise MessageHandleError("EXPLOSION_END can only be sent by the server")

    if isinstance(message, DeathMessage):
        print(f"Received DeathMessage from client {message.sender_id} for player {message.player}")
        raise MessageHandleError("DEATH can only be sent by the server")

    if isinstance(message, BonusAvailableMessage):
        print(f"Received BonusAvailableMessage from client {message.sender_id} with bonus type {message.bonus_type} at position ({message.position})")
        raise MessageHandleError("BONUS_AVAILABLE can only be sent by the server")

    if isinstance(message, BonusRetrievedMessage):
        print(f"Received BonusRetrievedMessage from client {message.sender_id} for player {message.player} at position ({message.position})")
        raise MessageHandleError("BONUS_RETRIEVED can only be sent by the server")

    if isinstance(message, BlockDestroyedMessage):
        print(f"Received BlockDestroyedMessage from client {message.sender_id} at position ({message.position})")
        raise MessageHandleError("BLOCK_DESTROYED can only be sent by the server")

    raise MessageHandleError(f"Unhandled message type: {type(message)}")


def handle_client(client_socket: socket.socket, addr):
    print(f"Handling client {addr}")
    client_socket.settimeout(10)  # Set a timeout for receiving data
    previous_data = b""
    client_id = None  # Will be set when we receive a HelloMessage from the client
    try:
        while True:
            data = client_socket.recv(1024)
            if not data:
                print(f"Client {addr} disconnected.")
                break

            has_leftover = True
            
            while has_leftover:
                has_leftover = False
                previous_data += data

                try:
                    message, leftover, needs_more = parse_message(previous_data)
                except ValueError as e:
                    print(f"Error parsing message from {addr}: {e}")
                    send_message(
                        client_socket,
                    ErrorMessage(sender_id=255, target_id=0, error_message=str(e))
                    )
                    break  # Exit the loop on parsing error

                if needs_more:
                    print(f"Waiting for more data from {addr} to complete the message.")
                    continue  # Wait for more data to complete the message

                assert message is not None, "Message should not be None if needs_more is False"

                previous_data = leftover  # Update previous_data with any leftover data
                has_leftover = len(leftover) > 0  # Check if there is leftover data to process

                if client_id is None and isinstance(message, HelloMessage):
                    client_id = message.sender_id  # Set client_id from the HelloMessage


                if isinstance(message, LeaveMessage):
                    print(f"Received LeaveMessage from client {message.sender_id}")
                    break

                handle_message(client_socket, message)  # Handle the message based on its type
    except socket.timeout:
        print(f"Client {addr} timed out.")

    except MessageHandleError as e:
        print(f"Message handling error for client {addr}: {e}")
        send_message(
            client_socket,
        ErrorMessage(sender_id=255, target_id=client_id if client_id is not None else 0, error_message=str(e))
        )

    except Exception as e:
        print(f"Error handling client {addr}: {e}")
    
    finally:
        client_socket.settimeout(None)  # Remove timeout
        try:
            send_message(
                client_socket,
            DisconnectMessage(sender_id=255, target_id=client_id if client_id is not None else 0)
            )
        except Exception as _:
            pass  # Ignore errors when trying to send disconnect message
        client_socket.close()
        print(f"Closed connection to {addr}")


server_socket = socket.socket(socket.AF_INET6, socket.SOCK_STREAM)
# Disable IPV6_V6ONLY to allow this socket to accept both IPv4 and IPv6 connections
server_socket.setsockopt(socket.IPPROTO_IPV6, socket.IPV6_V6ONLY, 0)
server_socket.bind(('::', 12345))  # Bind to all interfaces on port 12345
server_socket.listen(5)  # Listen for incoming connections

while True:
    client_socket, addr = server_socket.accept()  # Accept a connection
    client_socket.setsockopt(socket.SOL_SOCKET, socket.SO_KEEPALIVE, 1)  # Enable TCP keepalive
    client_socket.settimeout(60)  # Set a timeout for client operations
    client_socket.setsockopt(socket.IPPROTO_TCP, socket.TCP_NODELAY, 1)  # Disable Nagle's algorithm for lower latency
    print(f"Connection from {addr}, spawning thread to handle client.")
    client_thread = threading.Thread(target=handle_client, args=(client_socket, addr))
    client_thread.start()

