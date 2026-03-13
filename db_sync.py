#!/usr/bin/env python3
import sqlite3
import mysql.connector
from datetime import datetime
import json
import os

class DbSync:
    def __init__(self, sqlite_path, mysql_config):
        self.sqlite_path = sqlite_path
        self.mysql_config = mysql_config
        self.changes_file = "sync_changes.json"
        
    def get_sqlite_conn(self):
        return sqlite3.connect(self.sqlite_path)
    
    def get_mysql_conn(self):
        return mysql.connector.connect(**self.mysql_config)
    
    def get_sqlite_data(self):
        conn = self.get_sqlite_conn()
        cursor = conn.cursor()
        cursor.execute("SELECT TAG_ID, TAG_NAME, NODE_ID, NAMESPACE, TYPE_ID, TYPE_EXTRA, IO_ADDR, SCAN_INTV, DEVICE_ID FROM T_DD_SM_VAR")
        rows = cursor.fetchall()
        conn.close()
        return {row[0]: {
            'TAG_ID': row[0],
            'TAG_NAME': row[1],
            'NODE_ID': row[2],
            'NAMESPACE': row[3],
            'TYPE_ID': row[4],
            'TYPE_EXTRA': row[5],
            'IO_ADDR': row[6],
            'SCAN_INTV': row[7],
            'DEVICE_ID': row[8]
        } for row in rows}
    
    def get_mysql_data(self):
        conn = self.get_mysql_conn()
        cursor = conn.cursor()
        cursor.execute("SELECT TAG_ID, TAG_NAME, NODE_ID, NAMESPACE, TYPE_ID, TYPE_EXTRA, IO_ADDR, SCAN_INTV, DEVICE_ID FROM T_DD_SM_VAR")
        rows = cursor.fetchall()
        conn.close()
        return {row[0]: {
            'TAG_ID': row[0],
            'TAG_NAME': row[1],
            'NODE_ID': row[2],
            'NAMESPACE': row[3],
            'TYPE_ID': row[4],
            'TYPE_EXTRA': row[5],
            'IO_ADDR': row[6],
            'SCAN_INTV': row[7],
            'DEVICE_ID': row[8]
        } for row in rows}
    
    def compare_and_sync(self):
        time_stats = {}
        
        t0 = datetime.now()
        sqlite_data = self.get_sqlite_data()
        mysql_data = self.get_mysql_data()
        t1 = datetime.now()
        time_stats['fetch_time'] = (t1 - t0).total_seconds()
        
        changes = {
            'inserted': [],
            'updated': [],
            'deleted': []
        }
        
        sqlite_ids = set(sqlite_data.keys())
        mysql_ids = set(mysql_data.keys())
        
        new_ids = sqlite_ids - mysql_ids
        deleted_ids = mysql_ids - sqlite_ids
        common_ids = sqlite_ids & mysql_ids
        
        for tag_id in new_ids:
            changes['inserted'].append(sqlite_data[tag_id])
        
        for tag_id in deleted_ids:
            changes['deleted'].append(mysql_data[tag_id])
        
        for tag_id in common_ids:
            if sqlite_data[tag_id] != mysql_data[tag_id]:
                changes['updated'].append(sqlite_data[tag_id])
        
        t2 = datetime.now()
        time_stats['compare_time'] = (t2 - t1).total_seconds()
        
        t3 = datetime.now()
        self.apply_changes(changes)
        t4 = datetime.now()
        time_stats['sync_time'] = (t4 - t3).total_seconds()
        
        self.save_changes(changes, time_stats)
        
        return changes, time_stats
    
    @staticmethod
    def batch_process(data, batch_size, func):
        for i in range(0, len(data), batch_size):
            batch = data[i:i + batch_size]
            func(batch)
    
    def apply_changes(self, changes):
        conn = self.get_mysql_conn()
        cursor = conn.cursor()
        batch_size = 10000
        
        if changes['inserted']:
            insert_data = [
                (r['TAG_ID'], r['TAG_NAME'], r['NODE_ID'], r['NAMESPACE'],
                 r['TYPE_ID'], r['TYPE_EXTRA'], r['IO_ADDR'], r['SCAN_INTV'], r['DEVICE_ID'])
                for r in changes['inserted']
            ]
            self.batch_process(insert_data, batch_size, lambda batch: cursor.executemany("""
                INSERT INTO T_DD_SM_VAR 
                (TAG_ID, TAG_NAME, NODE_ID, NAMESPACE, TYPE_ID, TYPE_EXTRA, IO_ADDR, SCAN_INTV, DEVICE_ID)
                VALUES (%s, %s, %s, %s, %s, %s, %s, %s, %s)
            """, batch))
        
        if changes['updated']:
            update_data = [
                (r['TAG_NAME'], r['NODE_ID'], r['NAMESPACE'], r['TYPE_ID'],
                 r['TYPE_EXTRA'], r['IO_ADDR'], r['SCAN_INTV'], r['DEVICE_ID'], r['TAG_ID'])
                for r in changes['updated']
            ]
            self.batch_process(update_data, batch_size, lambda batch: cursor.executemany("""
                UPDATE T_DD_SM_VAR SET 
                TAG_NAME=%s, NODE_ID=%s, NAMESPACE=%s, TYPE_ID=%s, 
                TYPE_EXTRA=%s, IO_ADDR=%s, SCAN_INTV=%s, DEVICE_ID=%s
                WHERE TAG_ID=%s
            """, batch))
        
        if changes['deleted']:
            delete_data = [(r['TAG_ID'],) for r in changes['deleted']]
            self.batch_process(delete_data, batch_size, lambda batch: cursor.executemany("DELETE FROM T_DD_SM_VAR WHERE TAG_ID=%s", batch))
        
        conn.commit()
        conn.close()
    
    def save_changes(self, changes, time_stats=None):
        result = {
            'sync_time': datetime.now().isoformat(),
            'time_stats': time_stats or {},
            'inserted_ids': [r['TAG_ID'] for r in changes['inserted']],
            'updated_ids': [r['TAG_ID'] for r in changes['updated']],
            'deleted_ids': [r['TAG_ID'] for r in changes['deleted']],
            'inserted_records': changes['inserted'],
            'updated_records': changes['updated'],
            'deleted_records': changes['deleted']
        }
        with open(self.changes_file, 'w', encoding='utf-8') as f:
            json.dump(result, f, ensure_ascii=False, indent=2)
    
    def create_mysql_table(self):
        conn = self.get_mysql_conn()
        cursor = conn.cursor()
        cursor.execute("""
            CREATE TABLE IF NOT EXISTS T_DD_SM_VAR (
                TAG_ID INT UNSIGNED NOT NULL AUTO_INCREMENT COMMENT '唯一标识，自增',
                TAG_NAME VARCHAR(256) NOT NULL COMMENT '变量名称',
                NODE_ID INT UNSIGNED NOT NULL COMMENT '所属节点ID',
                NAMESPACE VARCHAR(64) NOT NULL COMMENT '命名空间',
                TYPE_ID VARCHAR(64) NOT NULL COMMENT '变量类型',
                TYPE_EXTRA JSON COMMENT '类型的补充信息',
                IO_ADDR TEXT COMMENT '地址',
                SCAN_INTV INT COMMENT '扫描周期',
                DEVICE_ID INT UNSIGNED COMMENT '设备ID',
                PRIMARY KEY (TAG_ID)
            ) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci COMMENT='变量表'
        """)
        conn.close()
    
    def create_sqlite_table(self):
        conn = self.get_sqlite_conn()
        cursor = conn.cursor()
        cursor.execute("""
            CREATE TABLE IF NOT EXISTS T_DD_SM_VAR (
                TAG_ID INTEGER PRIMARY KEY AUTOINCREMENT,
                TAG_NAME VARCHAR(256) NOT NULL,
                NODE_ID INTEGER NOT NULL,
                NAMESPACE VARCHAR(64) NOT NULL,
                TYPE_ID VARCHAR(64) NOT NULL,
                TYPE_EXTRA TEXT,
                IO_ADDR TEXT,
                SCAN_INTV INTEGER,
                DEVICE_ID INTEGER
            )
        """)
        conn.commit()
        conn.close()


if __name__ == "__main__":
    mysql_config = {
        'host': 'localhost',
        'port': 3306,
        'user': 'root',
        'password': 'mysql',
        'database': 'dbswitch'
    }
    
    sqlite_path = "/home/wzq/code/repos/dbswitch/T_DD_SM_VAR.db"
    
    syncer = DbSync(sqlite_path, mysql_config)
    
    # syncer.create_sqlite_table()
    # syncer.create_mysql_table()
    
    changes, time_stats = syncer.compare_and_sync()
    
    print(f"同步完成:")
    print(f"  新增: {len(changes['inserted'])}")
    print(f"  更新: {len(changes['updated'])}")
    print(f"  删除: {len(changes['deleted'])}")
    print(f"  数据获取耗时: {time_stats['fetch_time']:.3f}s")
    print(f"  数据对比耗时: {time_stats['compare_time']:.3f}s")
    print(f"  同步执行耗时: {time_stats['sync_time']:.3f}s")
    print(f"变更记录已保存到: sync_changes.json")
