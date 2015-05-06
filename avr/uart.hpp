class uart
{
	public:
		static void init(unsigned int ubrr);
		static void transmitByte(unsigned char data);
		static void transmitPacket(unsigned char data[], unsigned int size);
		static void transmitString(unsigned char* data);
		static void transmitString(const char* data);
		static unsigned char receiveByte(void);
};