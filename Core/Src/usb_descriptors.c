/**
 * @file           : usb_descriptors.c
 * @brief          : USB descriptors and string handling
 */

#ifndef TESTING
 #include "bsp/board_api.h"
 #include "tusb.h"
 #include "ump_discovery.h"
#else
 #include "bsp/board_api.h"
 #include "tusb.h"
 #include "ump_mocks.h"
#endif
 #include "mode_manager.h"
 #include <string.h>
 
 // Forward declaration
 uint8_t tud_alt_setting(uint8_t itf);
 
 /* A combination of interfaces must have a unique product id, since PC will save device driver after the first plug.
  * Same VID/PID with different interface e.g MSC (first), then CDC (later) will possibly cause system error on PC.
  *
  * Auto ProductID layout's Bitmap:
  *   [MSB]         HID | MSC | CDC          [LSB]
  */
 #define _PID_MAP(itf, n)  ( (CFG_TUD_##itf) << (n) )
 #define USB_PID           (0x4000 | _PID_MAP(CDC, 0) | _PID_MAP(MSC, 1) | _PID_MAP(HID, 2) | \
                            _PID_MAP(MIDI, 3) | _PID_MAP(VENDOR, 4) )

 
 //--------------------------------------------------------------------+
 // Device Descriptors
 //--------------------------------------------------------------------+

 #include "usb_descriptors.h"

// Default device descriptor (MIDI 2.0)
uint8_t const desc_device[] = {
	0x12,	// bLength
	DESC_TYPE_DEVICE,	// bDescriptorType = TUSB_DESC_DEVICE
	0x00,	// bcdUSBLSB
	0x02,	// bcdUSBMSB
	USB_CLASS_MISC,	// bDeviceClass = TUSB_CLASS_MISC
	MISC_SUBCLASS_COMMON,	// bDeviceSubClass = TUSB_CLASS_MISC
	MISC_PROTOCOL_IAD,	// bDeviceProtocol = MISC_PROTOCOL_IAD
	EP0_MAX_PACKET_SIZE,	// bMaxPacketSize0
	(USB_VID & 0xFF),	    // idVendorLSB
	((USB_VID >> 8) & 0xFF),	// idVendorMSB
	(USB_PID_MIDI20 & 0xFF),	    // idProductLSB
	((USB_PID_MIDI20 >> 8) & 0xFF),	// idProductMSB
	0x00,	// bcdDeviceLSB
	0x40,	// bcdDeviceMSB
	0x01,	// iManufacturer (USB_MANUFACTURER_STRING)
	0x02,	// iProduct (USB_PRODUCT_STRING_MIDI20)
	0x03,	// iSerialNumber (USB_SERIAL_STRING)
	0x01	// bNumConfigurations
};

uint8_t const desc_device_qualifier[] = {
	0x0A,	// bLength
	DESC_TYPE_DEVICE_QUALIFIER,	// bDescriptorType = TUSB_DESC_DEVICE_QUALIFIER
	0x00,	// bcdUSBLSB
	0x02,	// bcdUSBMSB
	USB_CLASS_MISC,	// bDeviceClass = TUSB_CLASS_MISC
	MISC_SUBCLASS_COMMON,	// bDeviceSubClass = TUSB_CLASS_MISC
	MISC_PROTOCOL_IAD,	// bDeviceProtocol = MISC_PROTOCOL_IAD
	EP0_MAX_PACKET_SIZE,	// bMaxPacketSize0
	0x01,	// bNumConfigurations
	0x00	// bReserved
};
 
 // Invoked when received GET DEVICE DESCRIPTOR
 // Application return pointer to descriptor
 uint8_t const * tud_descriptor_device_cb(void)
 {
   // Check MIDI mode and return appropriate device descriptor
   if (ModeManager_GetMode() == MIDI_MODE_1_0) {
     // MIDI 1.0 mode: return standard MIDI device descriptor
     static const tusb_desc_device_t desc_device_midi1 = {
       .bLength            = sizeof(tusb_desc_device_t),
       .bDescriptorType    = TUSB_DESC_DEVICE,
       .bcdUSB             = USB_BCD_USB_2_0,
       .bDeviceClass       = USB_CLASS_MISC,    // TUSB_CLASS_MISC - Required for IAD
       .bDeviceSubClass    = MISC_SUBCLASS_COMMON,    // MISC_SUBCLASS_COMMON
       .bDeviceProtocol    = MISC_PROTOCOL_IAD,    // MISC_PROTOCOL_IAD
       .bMaxPacketSize0    = CFG_TUD_ENDPOINT0_SIZE,

       .idVendor           = USB_VID,
       .idProduct          = USB_PID_MIDI10,  // MIDI 1.0
       .bcdDevice          = 0x0100,

       .iManufacturer      = 0x01,
       .iProduct           = 0x02,
       .iSerialNumber      = 0x03,

       .bNumConfigurations = 0x01
     };
     return (uint8_t const *) &desc_device_midi1;
   } else {
     // MIDI 2.0 mode: return MIDI 2.0 device descriptor
     return (uint8_t const *) &desc_device;
   }
 }

 
 //--------------------------------------------------------------------+
 // Configuration Descriptor
 //--------------------------------------------------------------------+
 
 enum
 {
   ITF_NUM_MIDI = 0,
   ITF_NUM_MIDI_STREAMING,
   ITF_NUM_TOTAL
 };
 
 #define CONFIG_TOTAL_LEN  (TUD_CONFIG_DESC_LEN + TUD_MIDI_DESC_LEN)
 
 // STM32F401 endpoint numbers
 #define EPNUM_MIDI_OUT   0x01
 #define EPNUM_MIDI_IN   0x01


 // Standard MIDI 1.0 configuration descriptor
 uint8_t const desc_fs_configuration_midi1[] = {
   // Config number, interface count, string index, total length, attribute, power in mA
   TUD_CONFIG_DESCRIPTOR(1, ITF_NUM_TOTAL, 0, CONFIG_TOTAL_LEN, 0x00, 100),

   // Interface number, string index, EP Out & EP In address, EP size
   TUD_MIDI_DESCRIPTOR(ITF_NUM_MIDI, 0, EPNUM_MIDI_OUT, (0x80 | EPNUM_MIDI_IN), 64)
 };
 
 // Invoked when received GET CONFIGURATION DESCRIPTOR
 // Application return pointer to descriptor
 // Descriptor contents must exist long enough for transfer to complete
 uint8_t const * tud_descriptor_configuration_cb(uint8_t index)
 {
   (void) index; // for multiple configurations

   // Check MIDI mode and return appropriate descriptor
   if (ModeManager_GetMode() == MIDI_MODE_1_0) {
     // MIDI 1.0 mode: return standard MIDI descriptor
     return desc_fs_configuration_midi1;
   } else {
     // MIDI 2.0 mode: return MIDI 2.0 descriptor with UMP
     return desc_fs_configuration;
   }
 }

 
 uint8_t const* tud_descriptor_device_qualifier_cb(void)
 {
  return (uint8_t const*) &desc_device_qualifier;
 }
 
 //--------------------------------------------------------------------+
 // String Descriptors
 //--------------------------------------------------------------------+
 
 // String Descriptor Index are now defined in header file
 
 static uint16_t _desc_str[32 + 1];
 
 // Invoked when received GET STRING DESCRIPTOR request
 // Application return pointer to descriptor, whose contents must exist long enough for transfer to complete
 uint16_t const *tud_descriptor_string_cb(uint8_t index, uint16_t langid) {
   (void) langid;
   size_t chr_count;
 
   switch ( index ) {
     case STRID_LANGID:
       memcpy(&_desc_str[1], string_desc_arr[0], 2);
       chr_count = 1;
       break;

     case STRID_PRODUCT:
       // Return different product name based on MIDI mode
       {
         const char* product_str = USB_GetProductString();
         size_t const max_count = sizeof(_desc_str) / sizeof(_desc_str[0]) - 1;
         chr_count = USB_ConvertASCIItoUTF16(product_str, &_desc_str[1], max_count);
       }
       break;
 
     case STRID_SERIAL:
       chr_count = board_usb_get_serial(_desc_str + 1, 32);
       break;
 
     default:
       // Note: the 0xEE index string is a Microsoft OS 1.0 Descriptors.
       // https://docs.microsoft.com/en-us/windows-hardware/drivers/usbcon/microsoft-defined-usb-descriptors
 
       if ( !(index < string_desc_arr_length) ) return NULL;
 
       const char *str = string_desc_arr[index];
       size_t const max_count = sizeof(_desc_str) / sizeof(_desc_str[0]) - 1;
       chr_count = USB_ConvertASCIItoUTF16(str, &_desc_str[1], max_count);
       break;
   }
 
   // first byte is length (including header), second byte is string type
   _desc_str[0] = (uint16_t) ((TUSB_DESC_STRING << 8) | (2 * chr_count + 2));
 
   return _desc_str;
 }
 
 //--------------------------------------------------------------------+
// Application Callback API Implementations
//--------------------------------------------------------------------+

// Invoked when audio class specific get request received for an interface
bool tud_ump_get_req_itf_cb(uint8_t rhport, tusb_control_request_t const * p_request)
{
  // Check request type - Group Terminal Block Descriptor request
  if (p_request->bRequest == 0x06 && p_request->wValue == ((CS_GR_TRM_BLOCK << 8) | GR_TRM_BLOCK_HEADER)) {
    // Return Group Terminal Block descriptor
    uint16_t length = p_request->wLength;
    uint16_t gtb_length = gtbLengths[0];  // Total length of GTB descriptor
    
    if (length > gtb_length) {
      length = gtb_length;
    }
    
    // Return the appropriate Group Terminal Block descriptor
    tud_control_xfer(rhport, p_request, (void *)group_descr[0], length);
    return true;
  }
  
  return false;  // Request not handled
}

// Invoked when MIDI streaming interface is mounted (alternate setting selected)
void tud_ump_mount_cb(uint8_t itf, uint8_t alt_setting)
{
  (void) itf;
  (void) alt_setting;
}

// Invoked when audio class specific get request received for an endpoint
bool tud_ump_get_req_ep_cb(uint8_t rhport, tusb_control_request_t const * p_request)
{
  (void) rhport;
  
  // For endpoint descriptor requests, wIndex contains the endpoint address
  uint8_t ep_addr = TU_U16_LOW(p_request->wIndex);
  
  // Check if this is a CS_ENDPOINT descriptor request
  if (p_request->bRequest == 0x06) {  // GET_DESCRIPTOR
    uint8_t desc_type = TU_U16_HIGH(p_request->wValue);
    
    if (desc_type == CS_ENDPOINT) {  // CS_ENDPOINT descriptor type
      // Get the interface number to determine alternate setting
      uint8_t itf_num = ITF_NUM_MIDI_STREAMING;
      uint8_t alt_setting = tud_alt_setting(itf_num);
      
      if (alt_setting == 1) {
        // MIDI 2.0 mode - return MS_GENERAL_2_0 descriptor
        const uint8_t ep_meta_midi2[] = {
          0x05,  // bLength
          CS_ENDPOINT,  // bDescriptorType = CS_ENDPOINT
          MS_GENERAL_2_0,  // bDescriptorSubtype = MS_GENERAL_2_0
          0x01,  // bNumGrpTrmBlock
          0x01   // baAssoGrpTrmBlkID
        };
        
        uint16_t length = TU_MIN(p_request->wLength, sizeof(ep_meta_midi2));
        return tud_control_xfer(rhport, p_request, (void*)ep_meta_midi2, length);
      } else {
        // MIDI 1.0 mode - return MS_GENERAL descriptor
        const uint8_t ep_meta_midi1[] = {
          0x05,  // bLength
          CS_ENDPOINT,  // bDescriptorType = CS_ENDPOINT
          MS_GENERAL,  // bDescriptorSubtype = MS_GENERAL
          0x01,  // bNumEmbMIDJack
          (ep_addr & 0x80) ? 0x12 : 0x01  // Jack ID based on direction
        };
        
        uint16_t length = TU_MIN(p_request->wLength, sizeof(ep_meta_midi1));
        return tud_control_xfer(rhport, p_request, (void*)ep_meta_midi1, length);
      }
    }
  }
  
  return false;  // Request not handled
}

// Define the MIDI 2.0 configuration descriptor
uint8_t const desc_fs_configuration[] = {
	0x09,	// bLength
	DESC_TYPE_CONFIGURATION,	// bDescriptorType = CONFIGURATION
	0x91,	// Total LengthLSB (145 bytes after reducing endpoint sizes)
	0x00,	// Total LengthMSB
	0x02,	// bNumInterfaces
	0x01,	// bConfigurationValue
	0x00,	// iConfiguration
	CONFIG_ATTR_BUS_POWERED,	// bmAttributes
	0x7D,	// bMaxPower (250mA)
	
  // ---------------------------
	
  // Interface Association Descriptor
	0x08,	// bLength
	DESC_TYPE_INTERFACE_ASSOCIATION,	// bDescriptorType
	0x00,	// bFirstInterface
	0x02,	// bInterfaceCount
	USB_CLASS_AUDIO,	// bFunctionClass
	AUDIO_SUBCLASS_STREAMING,	// bFunctionSubClass
	0x00,	// bFunctionProtocol
	0x00,	// iFunction
	
  // Interface - Audio Control
	0x09,	// bLength
	DESC_TYPE_INTERFACE,	// bDescriptorType = INTERFACE
	0x00,	// bInterfaceNumber
	0x00,	// bAlternateSetting
	0x00,	// bNumEndpoints
	USB_CLASS_AUDIO,	// bInterfaceClass = AUDIO
	AUDIO_SUBCLASS_CONTROL,	// bInterfaceSubClass = AUDIO_CONTROL
	0x00,	// bInterfaceProtocol
	0x00,	// iInterface
	
  // Audio AC Descriptor - Header
	0x09,	// bLength
	CS_INTERFACE,	// bDescriptorType = CS_INTERFACE
	AC_DESCRIPTOR_HEADER,	// bDescriptorSubtype = HEADER
	0x00,	// bcdACD0
	0x01,	// bcdACD1
	0x09,	// wTotalLengthLSB
	0x00,	// wTotalLengthMSB
	0x01,	// bInCollection
	0x01,	// baInterfaceNr(1)
	
  // Interface - MIDIStreaming - Alternate Setting #0
	0x09,	// bLength
	DESC_TYPE_INTERFACE,	// bDescriptorType = INTERFACE
	0x01,	// bInterfaceNumber
	0x00,	// bAlternateSetting
	0x02,	// bNumEndpoints
	USB_CLASS_AUDIO,	// bInterfaceClass = AUDIO
	AUDIO_SUBCLASS_STREAMING,	// bInterfaceSubClass = MIDISTREAMING
	0x00,	// bInterfaceProtocol
	0x05,	// iInterface (STRID_ITFNAME_ALT0)
	
  // Audio MS Descriptor - CS Interface - MS Header
	0x07,	// bLength
	CS_INTERFACE,	// bDescriptorType = CS_INTERFACE
	MS_DESCRIPTOR_HEADER,	// bDescriptorSubtype = MS_HEADER
	0x00,	// bcdMSCLSB
	0x01,	// bcdMSCMSB
	0x41,	// wTotalLengthLSB
	0x00,	// wTotalLengthMSB
	
  // Audio MS Descriptor - CS Interface - MIDI IN Jack (EMB) (Main In)
	0x06,	// bLength
	CS_INTERFACE,	// bDescriptorType = CS_INTERFACE
	MS_MIDI_IN_JACK,	// bDescriptorSubtype = MIDI_IN_JACK
	JACK_TYPE_EMBEDDED,	// bJackType = EMBEDDED
	0x01,	// bJackID (USB_INTERFACE_STRING)
	0x04,	// iJack (USB_INTERFACE_STRING)
	
  // Audio MS Descriptor - CS Interface - MIDI OUT Jack (EXT) (Main Out)
	0x09,	// bLength
	CS_INTERFACE,	// bDescriptorType = CS_INTERFACE
	MS_MIDI_OUT_JACK,	// bDescriptorSubtype = MIDI_OUT_JACK
	JACK_TYPE_EXTERNAL,	// bJackType = EXTERNAL
	0x01,	// bJackID for external (USB_INTERFACE_STRING)
	0x01,	// bNrInputPins
	0x01,	// baSourceID = Embedded bJackId (USB_INTERFACE_STRING)
	0x01,	// baSourcePin
	0x04,	// iJack (USB_INTERFACE_STRING)
	
  // Audio MS Descriptor - CS Interface - MIDI IN Jack (EXT) (Main In)
	0x06,	// bLength
	CS_INTERFACE,	// bDescriptorType = CS_INTERFACE
	MS_MIDI_IN_JACK,	// bDescriptorSubtype = MIDI_IN_JACK
	JACK_TYPE_EXTERNAL,	// bJackType = EXTERNAL
	0x02,	// bJackID for external (USB_INTERFACE_STRING)
	0x04,	// iJack (USB_INTERFACE_STRING)
	
  // Audio MS Descriptor - CS Interface - MIDI OUT Jack (EMB) (Main Out)
	0x09,	// bLength
	CS_INTERFACE,	// bDescriptorType
	MS_MIDI_OUT_JACK,	// bDescriptorSubtype
	JACK_TYPE_EMBEDDED,	// bJackType
	0x12,	// bJackID (USB_INTERFACE_STRING)
	0x01,	// Number of Input Pins of this Jack
	0x12,	// baSourceID (USB_INTERFACE_STRING)
	0x01,	// baSourcePin
	0x04,	// iJack (USB_INTERFACE_STRING)
	
  // EP Descriptor - Endpoint - MIDI OUT
	0x07,	// bLength (standard endpoint descriptor size)
	DESC_TYPE_ENDPOINT,	// bDescriptorType = ENDPOINT
	0x01,	// bEndpointAddress (OUT)
	EP_ATTR_BULK,	// bmAttributes
	FS_MAX_PACKET_SIZE,	// wMaxPacketSizeLSB
	0x00,	// wMaxPacketSizeMSB
	0x00,	// bInterval
	
  // Audio MS Descriptor - CS Endpoint - EP General
	0x05,	// bLength
	CS_ENDPOINT,	// bDescriptorType = CS_ENDPOINT
	MS_GENERAL,	// bDescriptorSubtype = MS_GENERAL
	0x01,	// bNumEmbMIDJack
	0x01,	// Jack Id - Embedded MIDI in (USB_INTERFACE_STRING)
	
  // EP Descriptor - Endpoint - MIDI IN
	0x07,	// bLength (standard endpoint descriptor size)
	DESC_TYPE_ENDPOINT,	// bDescriptorType = ENDPOINT
	0x81,	// bEndpointAddress (IN)
	EP_ATTR_BULK,	// bmAttributes
	FS_MAX_PACKET_SIZE,	// wMaxPacketSizeLSB
	0x00,	// wMaxPacketSizeMSB
	0x00,	// bInterval
	
  // Audio MS Descriptor - CS Endpoint - MS General
	0x05,	// bLength
	CS_ENDPOINT,	// bDescriptorType = CS_ENDPOINT
	MS_GENERAL,	// bDescriptorSubtype = MS_GENERAL
	0x01,	// bNumEmbMIDJack
	0x12,	// Jack Id - Embedded MIDI Out (USB_INTERFACE_STRING)
	
  // Interface - MIDIStreaming - Alternate Setting #1
	0x09,	// bLength
	DESC_TYPE_INTERFACE,	// bDescriptorType = INTERFACE
	0x01,	// bInterfaceNumber
	0x01,	// bAlternateSetting
	0x02,	// bNumEndpoints
	USB_CLASS_AUDIO,	// bInterfaceClass = AUDIO
	AUDIO_SUBCLASS_STREAMING,	// bInterfaceSubClass = MIDISTREAMING
	0x00,	//  bInterfaceProtocol
	0x06,	// iInterface (STRID_ITFNAME_ALT1)
	
  // Audio MS Descriptor - CS Interface - MS Header
	0x07,	// bLength
	CS_INTERFACE,	// bDescriptorType = CS_INTERFACE
	MS_DESCRIPTOR_HEADER,	// bDescriptorSubtype = MS_HEADER
	0x00,	// bcdMSC_LSB
	0x02,	// bcdMSC_MSB
	0x07,	// wTotalLengthLSB
	0x00,	// wTotalLengthMSB
	
  // EP Descriptor - Endpoint - MIDI OUT
	0x07,	// bLength (standard endpoint descriptor size)
	DESC_TYPE_ENDPOINT,	// bDescriptorType = ENDPOINT
	0x01,	// bEndpointAddress (OUT)
	EP_ATTR_BULK,	// bmAttributes
	FS_MAX_PACKET_SIZE,	// wMaxPacketSizeLSB
	0x00,	// wMaxPacketSizeMSB
	0x00,	// bInterval
	
  // Audio MS Descriptor - CS Endpoint - MS General 2.0
	0x05,	// bLength
	CS_ENDPOINT,	// bDescriptorType = CS_ENDPOINT
	MS_GENERAL_2_0,	// bDescriptorSubtype = MS_GENERAL_2_0
	0x01,	// bNumGrpTrmBlock
	0x01,	// baAssoGrpTrmBlkID
	
  // EP Descriptor - Endpoint - MIDI IN
	0x07,	// bLength (standard endpoint descriptor size)
	DESC_TYPE_ENDPOINT,	// bDescriptorType = ENDPOINT
	0x81,	// bEndpointAddress (IN)
	EP_ATTR_BULK,	// bmAttributes
	FS_MAX_PACKET_SIZE,	// wMaxPacketSizeLSB
	0x00,	// wMaxPacketSizeMSB
	0x00,	// bInterval
	
  // Audio MS Descriptor - CS Endpoint - MS General 2.0
	0x05,	// bLength
	CS_ENDPOINT,	// bDescriptorType = CS_ENDPOINT
	MS_GENERAL_2_0,	// bDescriptorSubtype = MS_GENERAL_2_0
	0x01,	// bNumGrpTrmBlock
	0x01	// baAssoGrpTrmBlkID
};

uint8_t const gtb0[] = {
	GTB_HEADER_LENGTH,	// HeaderLength
	CS_GR_TRM_BLOCK,	// bDescriptorType = CS_GR_TRM_BLOCK
	GR_TRM_BLOCK_HEADER,	// bDescriptorSubtype = GR_TRM_BLOCK_HEADER
	GTB_TOTAL_LENGTH,	// wTotalLengthLSB
	0x00,	// wTotalLengthMSB
	GTB_BLOCK_LENGTH,	// bLength
	CS_GR_TRM_BLOCK,	// bDescriptorType = CS_GR_TRM_BLOCK
	GR_TRM_BLOCK,	// bDescriptorSubtype = GR_TRM_BLOCK
	0x01,	// bGrpTrmBlkID
	GTB_TYPE_BIDIRECTIONAL,	// bGrpTrmBlkType = 0x00 (Bidirectional)
	FB0_FIRST_GROUP,	// nGroupTrm = First Group
	FB0_NUM_GROUPS,	// nNumGroupTrm = Number of groups
	0x04,	// iBlockItem (USB_INTERFACE_STRING)
	MIDI_PROTOCOL_2_0,	// bMIDIProtocol
	0x00,	// wMaxInputBandwidthLSB
	0x00,	// wMaxInputBandwidthMSB
	0x00,	// wMaxOutputBandwidthLSB
	0x00	// wMaxOutputBandwidthMSB
};

uint8_t const gtbLengths[] = {sizeof(gtb0)};
uint8_t const epInterface[] = {1};
uint8_t const *group_descr[] = {gtb0};
char const* string_desc_arr [] = {
	"", //0
	USB_MANUFACTURER_STRING, //1
	USB_PRODUCT_STRING_MIDI20, //2  // Default to MIDI 2.0 (will be dynamically changed)
	USB_SERIAL_STRING, //3
	USB_INTERFACE_STRING_ALT0, //4  // Interface name (also used for GTB)
	USB_INTERFACE_STRING_ALT0, //5
	USB_INTERFACE_STRING_ALT1, //6
};
uint8_t const string_desc_arr_length = sizeof(string_desc_arr) / sizeof(string_desc_arr[0]);

//--------------------------------------------------------------------+
// Testable String Helper Functions
//--------------------------------------------------------------------+

/**
 * @brief Get manufacturer string
 * @return Manufacturer string
 */
const char* USB_GetManufacturerString(void)
{
    return USB_MANUFACTURER_STRING;
}

/**
 * @brief Get product string based on current MIDI mode
 * @return Product string
 */
const char* USB_GetProductString(void)
{
    if (ModeManager_GetMode() == MIDI_MODE_1_0) {
        return USB_PRODUCT_STRING_MIDI10;
    } else {
        return USB_PRODUCT_STRING_MIDI20;
    }
}

/**
 * @brief Get serial number string
 * @return Serial number string
 */
const char* USB_GetSerialString(void)
{
    // For now, return the static serial string
    // In production, this would use board_usb_get_serial
    return USB_SERIAL_STRING;
}

/**
 * @brief Get interface name string
 * @return Interface name string
 */
const char* USB_GetInterfaceString(void)
{
    return USB_INTERFACE_STRING_ALT0;  // Default to ALT0 string
}

/**
 * @brief Convert ASCII string to UTF-16 for USB descriptors
 * @param ascii_str Input ASCII string
 * @param utf16_buf Output UTF-16 buffer
 * @param max_chars Maximum number of characters to convert
 * @return Number of characters converted
 */
size_t USB_ConvertASCIItoUTF16(const char* ascii_str, uint16_t* utf16_buf, size_t max_chars)
{
    if (!ascii_str || !utf16_buf || max_chars == 0) {
        return 0;
    }
    
    size_t chr_count = strlen(ascii_str);
    if (chr_count > max_chars) {
        chr_count = max_chars;
    }
    
    // Convert ASCII to UTF-16
    for (size_t i = 0; i < chr_count; i++) {
        utf16_buf[i] = (uint16_t)ascii_str[i];
    }
    
    return chr_count;
}

/**
 * @brief Get USB Vendor ID
 * @return Vendor ID (USB_VID for both MIDI modes)
 */
uint16_t USB_GetVendorID(void)
{
    return USB_VID;
}

/**
 * @brief Get USB Product ID based on current MIDI mode
 * @return Product ID (USB_PID_MIDI10 for MIDI 1.0, USB_PID_MIDI20 for MIDI 2.0)
 */
uint16_t USB_GetProductID(void)
{
    if (ModeManager_GetMode() == MIDI_MODE_1_0) {
        return USB_PID_MIDI10;  // MIDI 1.0
    } else {
        return USB_PID_MIDI20;  // MIDI 2.0
    }
}

