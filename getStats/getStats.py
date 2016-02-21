import mysql.connector
import time
import socket

db_config = {
    'user': 'gettagsd',
    'password': '',
    'host': '192.168.1.126',
    'database': 'nestBox',
}

graphite_ip = '192.168.1.126'
graphite_port = 2003


def setupDB(**_db_config):
    _dbx = mysql.connector.connect(
        **_db_config, buffered=True, autocommit=True)

    return _dbx


def setupSocket(_ip, _port):
    _socket = socket.socket()
    _socket.connect((_ip, _port))

    return _socket


def getScans(_interval, _dbx):
    _query = (
        "select s.scan_id, r.reader_id, r.reader_name, "
            "s.tag_id, s.created_date, t.tag_number "
        "from scans s, tags t, readers r "
        "where s.tag_id = t.tag_id and "
            "s.reader_id = r.reader_id and "
            "s.created_date > now() - interval %s week "
        "order by s.created_date asc")

    _cursor = _dbx.cursor()
    _cursor.execute(_query % _interval)
    _scans = _cursor.fetchall()
    _cursor.close()

    return _scans


def sendScan(_scan, _socket):
    _socket.sendall(_scan)

    print("sendScan: just sent %s" % _scan)
    return


dbx = setupDB(**db_config)
sock = setupSocket(graphite_ip, graphite_port)

go = True

while go:
    scans = getScans(3, dbx)
    # print(scans)

    for scan in scans:
        print(
            "nestBox.reader.scan %s %s %s %s %s %s" % (
                scan[0], scan[1], scan[2], scan[3],
                scan[4].timestamp(), scan[5]))

        per_tag_scan = "nestBox.%s.scan 1 %s\n" % (
                scan[3], scan[4].timestamp())
        per_reader_scan = "nestBox.%s.scan 1 %s\n" % (
                scan[2], scan[4].timestamp())
        per_reader_id_by_tag_id = "nestBox.%s.%s.scan 1 %s\n" % (
                scan[3], scan[2], scan[4].timestamp())
        per_tag_id_by_reader_id = "nestBox.%s.%s.scan 1 %s\n" % (
                scan[2], scan[3], scan[4].timestamp())

        print(per_tag_scan)
        print(per_reader_scan)
        print(per_reader_id_by_tag_id)
        print(per_tag_id_by_reader_id)

        sendScan(per_reader_scan.encode(), sock)
        sendScan(per_tag_scan.encode(), sock)
        sendScan(per_reader_id_by_tag_id.encode(), sock)
        sendScan(per_tag_id_by_reader_id.encode(), sock)
        # time.sleep(.1)
    go = False
dbx.close()
sock.close()
