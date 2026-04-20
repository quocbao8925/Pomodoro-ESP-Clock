#include "ui.h"
#include "pomodoro.h"
#include <stdio.h> // Cần thư viện này để dùng hàm sprintf

// Hàm vẽ giao diện Classic Squix
void draw_classic_squix(u8g2_t *u8g2, squix_ui_t data) {
    u8g2_ClearBuffer(u8g2);

    // --- 1. TÁCH CHUỖI GIỜ VÀ PHÚT ---
    // Giả sử data.time luôn có định dạng "HH:MM" (Ví dụ: "12:39")
    // Ta lấy 2 ký tự đầu làm Giờ, 2 ký tự cuối làm Phút
    char hour_str[3] = {data.time[0], data.time[1], '\0'};
    char min_str[3] = {data.time[3], data.time[4], '\0'};

    // --- 2. ĐƯỜNG KẺ DỌC PHÂN CÁCH ---
    // Kẻ một đường thẳng đứng tại X = 48
    u8g2_DrawVLine(u8g2, 52, 0, 64); 
    u8g2_DrawFrame(u8g2, 0, 0, 128, 64); // Viền ngoài
    // ==========================================
    // KHU VỰC BÊN TRÁI: GIỜ PHÚT NGĂN NẮP
    // ==========================================
    // Dùng font to vừa vặn với nửa màn hình (Cao 24px)
    u8g2_SetFont(u8g2, u8g2_font_logisoso24_tf); 
    
    // In Giờ (Nửa trên)
    u8g2_DrawStr(u8g2, 4, 28, hour_str);
    
    // In Phút (Nửa dưới)
    u8g2_DrawStr(u8g2, 4, 60, min_str);

    // In AM/PM nhỏ xíu nép vào góc phải của phút
    if (!data.is_24h) {
        u8g2_SetFont(u8g2, u8g2_font_5x8_tf); 
        // Đặt X=35 để vừa vặn lọt khe trước đường phân cách X=48
        u8g2_DrawStr(u8g2, 36, 60, data.is_pm ? "PM" : "AM");
    }

    // ==========================================
    // KHU VỰC BÊN PHẢI: THỜI TIẾT VÀ THÔNG TIN
    // ==========================================
    
    // 1. Icon Thời tiết (Đã thu nhỏ từ 4x xuống 2x)
    u8g2_SetFont(u8g2, u8g2_font_open_iconic_weather_2x_t);
    // Đặt ở X=58 (Sau đường kẻ), Y=24 (Nửa trên)
    u8g2_DrawGlyph(u8g2, 60, 24, data.icon); 

    // 2. Nhiệt độ siêu to (Nằm ngay dưới Icon)
    u8g2_SetFont(u8g2, u8g2_font_logisoso18_tf);
    char temp_buf[10];
    snprintf(temp_buf, 10, "%d", data.temp); // Dùng snprintf cho an toàn
    u8g2_DrawUTF8(u8g2, 58, 46, temp_buf);
        // Thêm ký tự độ C nhỏ xíu ngay sau số nhiệt độ
    u8g2_SetFont(u8g2, u8g2_font_5x8_tf);
    u8g2_DrawUTF8(u8g2, 88, 46, "°C");

    // 3. Thông tin phụ (Góc trên cùng bên phải)
    u8g2_SetFont(u8g2, u8g2_font_6x10_tf);
    u8g2_DrawStr(u8g2, 88, 14, data.date); // Ngày tháng
    u8g2_DrawStr(u8g2, 88, 26, data.city); // Thành phố
    
    // 4. Độ ẩm (Góc dưới cùng)
    char hum_buf[15];
    snprintf(hum_buf, 15, "H: %d%%", data.humidity);
    u8g2_DrawStr(u8g2, 58, 62, hum_buf);   

    u8g2_SendBuffer(u8g2);
}

// Hàm vẽ giao diện Smartwatch
void draw_smartwatch_ui(u8g2_t *u8g2, smartwatch_data_t data) {

    u8g2_ClearBuffer(u8g2);
    // Thêm AM/PM cho Smartwatch
    if (!data.is_24h) {
        u8g2_SetFont(u8g2, u8g2_font_5x8_tf); 
        u8g2_DrawStr(u8g2, 65, 18, data.is_pm ? "PM" : "AM");
    }
    // 1. Vẽ Giờ (Góc trên bên trái)
    u8g2_SetFont(u8g2, u8g2_font_logisoso18_tf); 
    u8g2_DrawStr(u8g2, 6, 18, data.time_str);

    // 2. Vẽ Ngày tháng (Góc trên bên phải)
    u8g2_SetFont(u8g2, u8g2_font_6x12_tf);       
    u8g2_DrawStr(u8g2, 85, 18, data.date_str);

    // 3. Đường kẻ ngang phân cách
    u8g2_DrawHLine(u8g2, 0, 22, 128);            
    
    // 4. Icon thời tiết (Góc dưới bên trái)
    u8g2_SetFont(u8g2, u8g2_font_open_iconic_weather_2x_t); 
    u8g2_DrawGlyph(u8g2, 4, 50, data.weather_icon); 

    // 5. Nhiệt độ (Ở giữa)
    u8g2_SetFont(u8g2, u8g2_font_logisoso18_tf); 
    char temp_str[10];
    sprintf(temp_str, "%d°C", data.temp);
    u8g2_DrawUTF8(u8g2, 22, 56, temp_str);       

    // 6. Thông tin phụ (Góc dưới bên phải)
    u8g2_SetFont(u8g2, u8g2_font_6x12_tf);       
    u8g2_DrawStr(u8g2, 85, 42, data.city);
    
    char hum_str[15];
    sprintf(hum_str, "H: %d%%", data.humidity);
    u8g2_DrawStr(u8g2, 85, 56, hum_str);

    u8g2_SendBuffer(u8g2);
}
// void draw_settings_vertical(u8g2_t *u8g2, int bat_percent, int buzzer_level, int buzzer_pitch, int settings_selection) {
//     bat_percent = (bat_percent == 100) ? 99 : bat_percent;
//     u8g2_ClearBuffer(u8g2);
    
//     // Xoay 90 độ (Hướng in chữ dọc từ trên xuống dưới)
//     u8g2_SetFontDirection(u8g2, 1); 

//     // Kẻ đường phân cách chuẩn của bạn
//     u8g2_DrawVLine(u8g2, 80, 0, 128); 

//     // ==========================================
//     // KHU VỰC 1: PIN (Giữ nguyên tọa độ tuyệt đối)
//     // ==========================================
//     u8g2_DrawFrame(u8g2, 90, 5, 24, 16);
//     u8g2_DrawBox(u8g2, 115, 9, 2, 8);

//     int fill_width = (bat_percent * 21) / 100;
//     u8g2_DrawBox(u8g2, 92, 7, fill_width, 12);

//     char bat_str[15];
//     snprintf(bat_str, 15, "%d", bat_percent); 
    
//     u8g2_SetFont(u8g2, u8g2_font_logisoso20_tf);
//     u8g2_DrawStr(u8g2, 90, 24, bat_str);

//     u8g2_SetFont(u8g2, u8g2_font_7x13B_tf);
//     u8g2_DrawStr(u8g2, 90, 52, "%");

//     // ==========================================
//     // KHU VỰC 2: MENU BUZZER CÓ MŨI TÊN CHỌN
//     // ==========================================
//     u8g2_SetFont(u8g2, u8g2_font_7x14B_tf);
//     u8g2_DrawStr(u8g2, 60, 5, "BUZZER");

//     u8g2_SetFont(u8g2, u8g2_font_6x12_tf);
//     // Tọa độ trục X của 3 dòng (Cách nhau 15 pixel)
//     int row_x[3] = {45, 30, 15};
//     int text_start_y = 10; // Chữ bắt đầu lùi vào Y=16 để nhường chỗ trống Y=0..15 cho mũi tên

//     // Dòng 0: VOLUME
//     char vol_str[15];
//     if (buzzer_level == 0) snprintf(vol_str, 15, "VOL: OFF");
//     else snprintf(vol_str, 15, "VOL: %d", buzzer_level);
//     u8g2_DrawStr(u8g2, row_x[0], text_start_y, vol_str);

//     // Dòng 1: PITCH (Độ cao)
//     char pitch_str[15];
//     snprintf(pitch_str, 15, "PITCH: %d", buzzer_pitch);
//     u8g2_DrawStr(u8g2, row_x[1], text_start_y, pitch_str);

//     // Dòng 2: TEST BEEP
//     u8g2_DrawStr(u8g2, row_x[2], text_start_y, "TEST BEEP");

//     // Vẽ mũi tên tam giác chỉ vào dòng đang được chọn
//     int sel_x = row_x[settings_selection];
    
//     // Đỉnh mũi tên (X-3, Y=12), hai cạnh đáy ở Y=5. 
//     // Mũi tên chĩa thẳng xuống dưới (theo chiều đọc chữ của Direction 1)
//     u8g2_DrawTriangle(u8g2, sel_x + 4, 2, sel_x, 2, sel_x + 2, 6);

//     u8g2_SetFontDirection(u8g2, 0); 
//     u8g2_SendBuffer(u8g2);
// }


void draw_pomodoro_vertical(u8g2_t *u8g2, pomodoro_t *pomo, bool rotate, int loading_pct) {
    
    u8g2_ClearBuffer(u8g2);
    u8g2_DrawFrame(u8g2, 0, 0, 128, 64); // Viền ngoài
    u8g2_SetFontDirection(u8g2, (rotate) ? 3 : 1); // Xoay nếu cần thiết
    
    // --- 1. TIÊU ĐỀ (WORK / BREAK) ---
    // Hiển thị to ở góc trên cùng bên trái
    if (rotate) {    
        u8g2_SetFont(u8g2, u8g2_font_7x14B_tf);
        
        u8g2_DrawStr(u8g2, 110, 60, "WORKING");

        // Chữ POMODORO nhỏ nằm phía dưới tiêu đề (Góc trên bên phải)
        u8g2_SetFont(u8g2, u8g2_font_5x8_tf);
        u8g2_DrawStr(u8g2, 95, 60, "STATUS:");
        u8g2_SetFont(u8g2, u8g2_font_5x8_tf);
        u8g2_DrawStr(u8g2, 120, 60, "POMODORO");

        // --- 2. MENU MỐC THỜI GIAN (Bên trái màn hình) ---
        u8g2_SetFont(u8g2, u8g2_font_6x12_tf);
        u8g2_DrawStr(u8g2, 60, 18, " 5");
        u8g2_DrawStr(u8g2, 40, 18, "25");

        // Tính toán vị trí trục X cho mũi tên tam giác
        int arrow_x = 40;

        // Vẽ mũi tên tam giác chỉ vào mốc đang chọn
        u8g2_DrawTriangle(u8g2, arrow_x - 4, 1, arrow_x + 4, 1, arrow_x, 5);

        // --- 3. ĐỒNG HỒ ĐẾM NGƯỢC CHÍNH ---
        u8g2_SetFont(u8g2, u8g2_font_logisoso22_tf);
        
        char min_buf[5];
        sprintf(min_buf, "%02d", pomo->minutes);
        u8g2_DrawStr(u8g2, 30, 62, min_buf); // Dịch Phút lên một chút

        char sec_buf[5];
        sprintf(sec_buf, "%02d", pomo->seconds);
        u8g2_DrawStr(u8g2, 60, 62, sec_buf); // Dịch Giây xuống một chút

        if (loading_pct > 0 && loading_pct <= 100) {

        // --- CẤU HÌNH 3 KHỐI HỘP ---
            int box_w = 2;  // Chiều rộng mỗi hộp
            int box_h = 8;  // Chiều cao mỗi hộp
            int space = 2;   
            int start_x = 72; 
            int start_y = 10; 

            if (loading_pct > 0) {
                    int p1 = (loading_pct > 33) ? 33 : loading_pct;
                    int fill_h1 = (p1 * box_h) / 33; 
                    u8g2_DrawBox(u8g2, start_x, start_y, box_w, fill_h1);
                }
                
                // Hộp 2 (Giây thứ 2): Chạy từ 33% -> 66%
            if (loading_pct > 33) {
                int p2 = (loading_pct > 66) ? 33 : (loading_pct - 33);
                int fill_h2 = (p2 * box_h) / 33;
                u8g2_DrawBox(u8g2, start_x + box_w + space, start_y, box_w, fill_h2);
            }

            // Hộp 3 (Giây thứ 3): Chạy từ 66% -> 100%
            if (loading_pct > 66) {
                int p3 = (loading_pct > 100) ? 34 : (loading_pct - 66);
                int fill_h3 = (p3 * box_h) / 34;
                u8g2_DrawBox(u8g2, start_x + (box_w + space)*2, start_y, box_w, fill_h3);
            }
        }

        // --- 4. ANIMATION (Trở về góc 0 độ) ---
        u8g2_SetFontDirection(u8g2, 0); 

        // Dấu chấm nhấp nháy
        if (pomo->seconds % 2 == 0) {
            u8g2_DrawDisc(u8g2, 80, 62, 3, U8G2_DRAW_ALL); 
        }

        // Thanh tiến trình chạy dọc mép dưới
        int bar_length = (pomo->seconds * 64) / 60; 
        u8g2_DrawBox(u8g2, 0, 0, 3, bar_length); 

        if (pomo->flash_timer > 0) {
            // Cứ mỗi 2 nhịp (200ms) thì đảo màu 1 lần
            if ((pomo->flash_timer / 2) % 2 == 0) {
                u8g2_SetDrawColor(u8g2, 2);        // MÀU 2: Chế độ XOR (Đảo ngược pixel)
                u8g2_DrawBox(u8g2, 0, 0, 128, 64); // Vẽ một khối hộp đen kịt phủ kín toàn bộ màn hình
                u8g2_SetDrawColor(u8g2, 1);        // Trả lại bút vẽ về MÀU 1 (Màu trắng) để vẽ bình thường
            }
        }

        u8g2_SendBuffer(u8g2); // Lệnh gửi dữ liệu ra màn hình
    } else {

        u8g2_SetFont(u8g2, u8g2_font_7x14B_tf);

        u8g2_DrawStr(u8g2, 18, 4, "BREAKING");

        // Chữ POMODORO nhỏ nằm phía dưới tiêu đề (Góc trên bên phải)
        u8g2_SetFont(u8g2, u8g2_font_5x8_tf);
        u8g2_DrawStr(u8g2, 33, 4, "STATUS:");
        u8g2_SetFont(u8g2, u8g2_font_5x8_tf);
        u8g2_DrawStr(u8g2, 8, 4, "POMODORO");

        // --- 2. MENU MỐC THỜI GIAN (Bên trái màn hình) ---
        u8g2_SetFont(u8g2, u8g2_font_6x12_tf);
        u8g2_DrawStr(u8g2, 68, 46, " 5");
        u8g2_DrawStr(u8g2, 88, 46, "25");

        // Tính toán vị trí trục X cho mũi tên tam giác
        int arrow_x = 60;
        // Vẽ mũi tên tam giác chỉ vào mốc đang chọn
        u8g2_DrawTriangle(u8g2, 132 - arrow_x, 62, 124 - arrow_x, 62, 128 - arrow_x, 59);

        // --- 3. ĐỒNG HỒ ĐẾM NGƯỢC CHÍNH ---
        u8g2_SetFont(u8g2, u8g2_font_logisoso22_tf);
        
        char min_buf[5];
        sprintf(min_buf, "%02d", pomo->minutes);
        u8g2_DrawStr(u8g2, 98, 2, min_buf); // Dịch Phút lên một chút

        char sec_buf[5];
        sprintf(sec_buf, "%02d", pomo->seconds);
        u8g2_DrawStr(u8g2, 68, 2, sec_buf); // Dịch Giây xuống một chút

        if (loading_pct > 0 && loading_pct <= 100) {

        // --- CẤU HÌNH 3 KHỐI HỘP ---
            int box_w = 2;  // Chiều rộng mỗi hộp
            int box_h = 8;  // Chiều cao mỗi hộp
            int space = 2;   
            int start_x = 48; 
            int start_y = 46; 

            if (loading_pct > 0) {
                    int p1 = (loading_pct > 33) ? 33 : loading_pct;
                    int fill_h1 = (p1 * box_h) / 33; 
                    u8g2_DrawBox(u8g2, start_x, start_y, box_w, fill_h1);
                }
                
                // Hộp 2 (Giây thứ 2): Chạy từ 33% -> 66%
            if (loading_pct > 33) {
                int p2 = (loading_pct > 66) ? 33 : (loading_pct - 33);
                int fill_h2 = (p2 * box_h) / 33;
                u8g2_DrawBox(u8g2, start_x + box_w + space, start_y, box_w, fill_h2);
            }

            // Hộp 3 (Giây thứ 3): Chạy từ 66% -> 100%
            if (loading_pct > 66) {
                int p3 = (loading_pct > 100) ? 34 : (loading_pct - 66);
                int fill_h3 = (p3 * box_h) / 34;
                u8g2_DrawBox(u8g2, start_x + (box_w + space)*2, start_y, box_w, fill_h3);
            }
        }

        // --- 4. ANIMATION (Trở về góc 0 độ) ---
        u8g2_SetFontDirection(u8g2, 0); 

        // Dấu chấm nhấp nháy
        if (pomo->seconds % 2 == 0) {
            u8g2_DrawDisc(u8g2, 48, 1, 3, U8G2_DRAW_ALL); 
        }

        // Thanh tiến trình chạy dọc mép dưới
        int bar_length = (pomo->seconds * 64) / 60; 
        u8g2_DrawBox(u8g2, 125, 64 - bar_length, 3, bar_length); 

        if (pomo->flash_timer > 0) {
            // Cứ mỗi 2 nhịp (200ms) thì đảo màu 1 lần
            if ((pomo->flash_timer / 2) % 2 == 0) {
                u8g2_SetDrawColor(u8g2, 2);        // MÀU 2: Chế độ XOR (Đảo ngược pixel)
                u8g2_DrawBox(u8g2, 0, 0, 128, 64); // Vẽ một khối hộp đen kịt phủ kín toàn bộ màn hình
                u8g2_SetDrawColor(u8g2, 1);        // Trả lại bút vẽ về MÀU 1 (Màu trắng) để vẽ bình thường
            }
        }

        u8g2_SendBuffer(u8g2); // Lệnh gửi dữ liệu ra màn hình
    }
       
}