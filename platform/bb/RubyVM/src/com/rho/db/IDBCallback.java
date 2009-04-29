package com.rho.db;

public interface IDBCallback 
{
	public abstract void OnDeleteAll();
	public abstract void OnDeleteAllFromTable(String tableName);
	public abstract void OnDeleteFromTable(String tableName, Object rows2Delete);

}