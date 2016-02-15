# designed to run as a daemon
# and store into a DB each
# time an RFID tag is scanned

import serial
import mysql.connector
import time
import re


def setupDB(**_db_config):
    _dbx = mysql.connector.connect(
        **_db_config, buffered=True, autocommit=True)

    return _dbx


def setupSerial(_serial):
    _ser = serial.Serial(_serial, 9600)
    # ser = serial.Serial('/dev/cu.Bluetooth-Incoming-Port', 9600)

    return _ser


def insertNewTag(_tag_number, _dbx):
    _insert_tag = (
        "insert into tags(tag_number) values('%s')")

    _cursor = _dbx.cursor()
    # _cursor.execute(_insert_tag % _tag_number.decode('ascii').rstrip())
    _cursor.execute(_insert_tag % _tag_number)
    _tag_id = _cursor.lastrowid
    _cursor.close()

    print("insertNewTag: ", tag_number, _tag_id)
    return _tag_id


def insertScan(_tag_id, _reader_id, _dbx):
    _insert_scan = (
        "insert into scans (tag_id, reader_id) values (%s, %s)")
    _cursor = _dbx.cursor()
    _cursor.execute(_insert_scan % (_tag_id, _reader_id))
    _scan_id = _cursor.lastrowid
    _cursor.close()
    print("insertScan: ", _scan_id, _tag_id, _reader_id)
    return _scan_id


def getReaderID(_serial_number, _dbx):
    _get_reader_id = (
        "select reader_id from readers where name = '%s'")

    _cursor = _dbx.cursor()
    _cursor.execute(_get_reader_id % _serial_number)
    _reader_id = _cursor.fetchone()
    _cursor.close()

    print("getReaderID: ", _serial_number, _reader_id)

    if _reader_id is None:
        return None

    else:
        return _reader_id[0]


def getTagID(_tag_number, _dbx):
    _tag_exists = (
        "select tag_id from tags "
        "where tag_number = '%s'")

    _cursor = _dbx.cursor()
    _cursor.execute(_tag_exists % _tag_number)
    _dbx.commit()
    _tag_id = _cursor.fetchone()
    _cursor.close()

    print("getTagID: ", _tag_number, _tag_id)

    if _tag_id is None:
        return None

    else:
        return _tag_id[0]


db_config = {
    'user': 'gettagsd',
    'password': '',
    'host': '192.168.1.126',
    'database': 'nestBox',
}

# setup the connections to DB and serial
dbx = setupDB(**db_config)
ser = setupSerial('/dev/cu.usbmodem1421')

while True:
    try:
        tag_number = ser.readline()

    except serial.SerialException:
        print("Serial port not ready...")
        time.sleep(1)
        continue

    tag_string = tag_number.decode('ascii').rstrip()
    fields = tag_string.split(",")

    for field in fields:
        if re.match(r"^2[0-9]{24}3$", field):
            tag_number = field
        else:
            serial_number = field

    try:
        tag_id = getTagID(tag_number, dbx)

    except mysql.connector.errors.ProgrammingError:
        print("%s is not a valid tag_number" % tag_number)
        continue

    if tag_id is None:
        tag_id = insertNewTag(tag_number, dbx)

    reader_id = getReaderID(serial_number, dbx)

    if reader_id is None:
        print("%s is an invalid reader_id" % reader_id)
        continue

    scan_id = insertScan(tag_id, reader_id, dbx)

dbx.close()
