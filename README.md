Process Listing
===

![](https://img.shields.io/badge/os-windows-yellow)
![](https://img.shields.io/badge/laguage-c%2B%2B-green)
![](https://img.shields.io/badge/mode-high--level--code-red)
![](https://img.shields.io/badge/libary-psapi.h-blue)
![](https://img.shields.io/badge/libary-winternl.h-blue)
![](https://img.shields.io/badge/libary-tlhelp32.h-blue)

## Table of Content <a name="menu"></a>

* [Table of Content](#menu)
* [Overall](#overall)
* [PS-API](#psapi)
* [Tool Help Libary](#toolhelp)
* [Windows Internal](#winternl)

## Overall <a name="overall"></a>

Đây là 1 project c++ về process listing (walking) trên hệ điều hành windows sử dụng 3 cách thức khác nhau:
* Process Status API (psapi.h): thư viện ở user-level hỗ trợ nhiều hàm để trích xuất thông tin tổng quan về process, thread, device information,... 
* Tool Help Libary (tlhelp32.h): chứa các hàm để chụp snapshot của system, sau đó dùng 1 số hàm khác để hỗ trợ truy xuất thông tin từ snapshot đó
* Window Internal (winternl.h): thư viện ở kernel-level code, là 1 thư viện mạnh, hỗ trợ truy xuất thông tin về System Services, Security và Identity,... dưới đặc quyền system.

### PS-API <a name="psapi"></a>

Vì đây là thư viện ở user-level nên có rất nhiều hạn chế về quyền hạn khi tương tác với các object của OS (process, thread, file, token,...). Tuy nhiên, nó cũng hỗ trợ rất nhiều hàm để truy xuất các thông tin về process. Với thư viện này, chúng ta sẽ dùng hàm EnumProcess() để lấy danh sách pid của toàn bộ các process đang chạy trên OS, sau đó dùng Openprocess() để lấy về handle của process với pid chỉ định.

![](https://i.imgur.com/JpJdXSQ.png)

> *Lưu ý: đối với các Protected Process (hay Protected Process Light) là các process object thuộc quyền system thì Openprocess() sẽ fail và trả về error code (phần lớn là 5: access denied). Trong code mình có cả hàm AdjustTokenPrivileges() để enable privilege lên (cụ thể là debug privilege), tuy nhiên Openprocess() vẫn fail:*

![](https://i.imgur.com/gKrMr2g.png)

Docs về PS-API: https://docs.microsoft.com/en-us/windows/win32/api/_psapi/

Về Protected Process (PPL), token, privileges,... các bạn có thể đọc thêm book **Windows Internal 7th Edition**


### Tool Help Libary <a name="toolhelp"></a>

Thư viện này chỉ hỗ trợ chụp ảnh snapshot của OS trong thời điểm gọi hàm CreateToolhelp32Snapshot(), rồi dùng 1 số hàm travel qua các module, process, thread entry để thu thập các thông tin như process name, executable path,... Nó không hỗ trợ trích xuất quá sâu vào các object vì đơn thuần nó chỉ chụp được snapshot thôi chứ không tương tác được với object đó (khác với psapi):

![](https://i.imgur.com/6AdP8lc.png)

> *Vì chỉ chụp snapshot thôi nên không thể tác động gì tới các object, do đó nó có thể in ra 1 số thông tin cơ bản của các protected process như: System Idle Process, Registry,...*


### Windows Internal <a name="winternl"></a>

Đây là 1 thư viện mạnh, hoạt động dưới quyền system nên nó có thể trích xuất sâu vào các system object. Chúng ta sẽ sử dụng hàm Ntquerysysteminformation(), truyền vào tham số là các System_Information_Class (có thể tham khảo trên msdocs hoặc winternl.h) để gửi câu truy vấn tới system và lấy về các thông tin mềm như pid, parent pid, image name, base priority, virtual memory size,...

![](https://i.imgur.com/vbjc3rO.png)

Trước đây, các hàm và struct của Ntqueysysteminformation có rất ít tài liệu về chúng, giờ đây thì msdocs đã lo việc đó rồi: https://docs.microsoft.com/en-us/windows/win32/api/winternl/nf-winternl-ntquerysysteminformation


