#include "database.h"

database::database():
    driver(sql::mysql::get_driver_instance()),
    /* Using the Driver to create a connection */
    con(driver->connect("localhost", "webdev", "guL9toh3le"))
{
    con->setSchema("bpla");
    log_norm() << "mysql connect";
    /*last_read_command = 0;*/
    unsent_commands_pstmt = con->prepareStatement(
            "select * from commands where sent is null");
}

database::~database()
{
    delete unsent_commands_pstmt;
}

unique_ptr<sql::ResultSet>
database::query(string q)
{
    unique_ptr<sql::ResultSet> res;
	try {
        unique_ptr<sql::Statement> stmt(con->createStatement());
        res.reset(stmt->executeQuery(q));
	} catch (sql::SQLException &e) {
        log_err() << e.what();
    }

    return res;
}

base_command*
database::command_poll()
{
    log_norm() << "Database command polling started";
    base_command* cmd;
    enum msg_dcp_types type;
    sql::ResultSet* res = unsent_commands_pstmt->executeQuery();
    if (res->wasNull())
    {
        log_norm() << "No unsent messages";
    }
    while (res->next())
    {
        type = static_cast<enum msg_dcp_types>(res->getUInt("type"));
        unsigned int num = res->getUInt("num");
        log_norm() << "Command " << num << ", type " << type;

        switch (type)
        {
            case Msg_NewRoute:
                /* poll route table */
                //cmd = newRoute;
                break;
            case Msg_CleanRoute:
                cmd = new cleanRoute;
                break;
            case Msg_UpdatePoint:
                break;
            case Msg_Emergency:
                break;
            case Msg_HandOn:
                cmd = new setManualMode;
                break;
            case Msg_HandOff:
                cmd = new setAutomaticMode;
                break;
            case Msg_ZeroBaroAlt:
                break;
            default: 
                log_err() << "Command " 
                          << num 
                          << ": invalid type" 
                          << type;
        }
    }
    return cmd;
}
