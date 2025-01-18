import cv2
import numpy as np

# Загрузка эталонного кадра (с чистой камеры)
reference_image = cv2.imread("reference_image.jpg")

# Функция для определения степени загрязнения
def get_pollution_level(image):
  # Преобразование изображений в оттенки серого
  gray_image = cv2.cvtColor(image, cv2.COLOR_BGR2GRAY)
  gray_reference = cv2.cvtColor(reference_image, cv2.COLOR_BGR2GRAY)

  # Вычисление абсолютного различия между изображениями
  diff = cv2.absdiff(gray_image, gray_reference)

  # Пороговое значение для выделения областей загрязнения
  thresh = cv2.threshold(diff, 20, 255, cv2.THRESH_BINARY)[1]

  # Подсчет количества пикселей загрязнения
  pollution_pixels = np.sum(thresh == 255)

  # Вычисление уровня загрязнения (в процентах)
  pollution_level = pollution_pixels / (image.shape[0] * image.shape[1]) * 100

  return pollution_level, thresh

# Загрузка текущего кадра
current_image = cv2.imread("current_image.jpg")

# Определение степени загрязнения и выделенных областей
pollution_level, thresh = get_pollution_level(current_image)

# Объединение выделенных областей с текущим кадром
result_image = cv2.bitwise_or(current_image, current_image, mask=thresh)

# Вывод результата
print(f"Уровень загрязнения: {pollution_level:.2f}%")

# Отображение текущего кадра с выделенными областями
cv2.imshow("Текущий кадр с областями загрязнения", result_image)
cv2.waitKey(0)
