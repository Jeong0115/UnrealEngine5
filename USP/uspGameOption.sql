CREATE DEFINER=`root`@`localhost` PROCEDURE `SelectUserGameOption`
(
    IN  inUserID    INT
)
BEGIN
    IF EXISTS (SELECT 1 FROM GameOptions WHERE UserID = inUserID) THEN
        SELECT 0 As Status;
        SELECT InventorySize
        FROM GameOptions WHERE UserID = inUserID;
    ELSE
        SELECT 1 As Status;
        SELECT "Cannot find user id" As Message;
    END IF;
END