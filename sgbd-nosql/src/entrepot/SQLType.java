package entrepot;

import java.io.Serializable;
import java.sql.Types;

public enum SQLType implements Serializable {

    ARRAY("ARRAY", false, Types.ARRAY), BIGINT("BIGINT", true, Types.BIGINT), BINARY("CHAR FOR BIT DATA", true, Types.BINARY),
    BIT("CHAR FOR BIT DATA", true, Types.BIT),
    BLOB("BLOB", false, Types.BLOB), CHAR("CHAR(50)", false, Types.CHAR), CLOB("CLOB", false, Types.CLOB), DATE("DATE", false, Types.DATE), 
    DECIMAL("DECIMAL", true, Types.DECIMAL),
    DOUBLE("DOUBLE PRECISION", true, Types.DOUBLE), INTEGER("INTEGER", true, Types.INTEGER), LONGVARBINARY("LONG VARCHAR FOR BIT DATA", false, Types.LONGVARBINARY),
    LONGVARCHAR("LONG VARCHAR", false, Types.LONGNVARCHAR), NUMERIC("DECIMAL", true, Types.NUMERIC), REAL("REAL", true, Types.REAL), SMALLINT("SMALLINT", true, Types.SMALLINT),
    SQLXML("XML", false, Types.SQLXML), TIME("TIME", false, Types.TIME), TIMESTAMP("TIMESTAMP", false, Types.TIMESTAMP), VARCHAR("VARCHAR(50)", false, Types.VARCHAR),
    VARCHAR512("VARCHAR(512)", false, Types.VARCHAR),
    NULL("", false, Types.NULL);
    
    
    private final String valeur;
    private final boolean isNum;
    private final int typeSql;

    private SQLType(String valeur, boolean b, int t) {
        this.valeur = valeur;
        this.isNum = b;
        this.typeSql = t;
    }

    public static SQLType getSQLType(int t) {
        switch (t) {
            case Types.ARRAY:
                return SQLType.ARRAY;
            case Types.BIGINT:
                return SQLType.BIGINT;
            case Types.BINARY:
                return SQLType.BINARY;
            case Types.BIT:
                return SQLType.BIT;
            case Types.BLOB:
                return SQLType.BLOB;
            case Types.CHAR:
                return SQLType.CHAR;
            case Types.CLOB:
                return SQLType.CLOB;
            case Types.DATE:
                return SQLType.DATE;
            case Types.DECIMAL:
                return SQLType.DECIMAL;
            case Types.DOUBLE:
                return SQLType.DOUBLE;
            case Types.INTEGER:
                return SQLType.INTEGER;
            case Types.LONGVARBINARY:
                return SQLType.LONGVARBINARY;
            case Types.LONGVARCHAR:
                return SQLType.LONGVARCHAR;
            case Types.NUMERIC:
                return SQLType.NUMERIC;
            case Types.REAL:
                return SQLType.REAL;
            case Types.SMALLINT:
                return SQLType.SMALLINT;
            case Types.SQLXML:
                return SQLType.SQLXML;
            case Types.TIME:
                return SQLType.TIME;
            case Types.TIMESTAMP:
                return SQLType.TIMESTAMP;
            case Types.VARCHAR:
                return SQLType.VARCHAR;
            case Types.NULL:
                return SQLType.NULL;
            default:
                return SQLType.NULL;
        }
    }

    public String getSQLType() {
        return valeur;
    }
    
    public int getTypeJSql () {
    	return typeSql;
    }

    public boolean isNumeric() {
        return isNum;
    }
}
