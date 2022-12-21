# Курсовой проект по дисциплине <Программирование встроенных приложений>
# На тему "Калькулятор" 
## [@xincas](https://github.com/xincas) и [@qveex](https://github.com/qveex)
![](https://github.com/xincas/stm32-touchscreen-calculator/blob/main/calc.gif)
## [Отчет](https://github.com/xincas/stm32-touchscreen-calculator/blob/main/%D0%9F%D0%92%D0%9F_%D0%9A%D0%9F_%D0%97%D0%B8%D0%BD%D0%B0%D1%82%D0%BE%D0%B2_%D0%9D%D0%B5%D1%81%D1%82%D0%B5%D1%80%D0%B5%D0%BD%D0%BA%D0%BE.pdf)
## В программе используется:
- **STM32F303VCT6**
- **Touch screen ILI9325**
- **[lvgl 7.11](https://lvgl.io/)**
- **Backup register / FLASH memory**

### Функции:
1. Вычисление основных арифметических операций над числами с плавающий запятой типа float;
2. Для считываний нажатий используется Touch Screen;
3. Чтение / Запись числа в память и возможность оперировать им при вычислениях;
4. Наличие режимов работы: «Серия равенств» (повторение последней операции при неоднократном нажатии клавиши «=» на дисплее) и «Серия операций» (при непрерывном вводе операций без ввода знака равенства для подтверждения вычисления.

## Диаграмма вариантов использования
![](https://github.com/xincas/stm32-touchscreen-calculator/blob/main/flow2-%D0%92%D0%B0%D1%80%D0%B8%D0%B0%D0%BD%D1%82%D1%8B%20%D0%B8%D1%81%D0%BF%D0%BE%D0%BB%D1%8C%D0%B7%D0%BE%D0%B2%D0%B0%D0%BD%D0%B8%D1%8F.drawio.png)
