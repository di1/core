(defvar :name "Marubuzu")
(defun main
  (name, chart)
  ((if
    (<=
      (chart-length chart) 0)
    (abort))
  (defvar last-candle
    (chart-get-candle chart 
      (- chart-length 1)))
  (if
    (=
      (candle-close last-candle)
      (candle-high last-candle)))))
