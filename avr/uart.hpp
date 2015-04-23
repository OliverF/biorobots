class uart
{
	public:
		static void init(unsigned int ubrr);
		static void transmitByte(unsigned char data);
		static void transmitPacket(unsigned char data[], unsigned int size);
		static unsigned char receiveByte(void);
};