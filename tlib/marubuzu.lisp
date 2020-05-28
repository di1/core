; let the interpreter know the name of the application and the type.
(set :name marubuzu)
; let the interpreter know that this is an analysis and not a strategy.
(set :type analysis)

; the main function takes in no parameters.
(defun main
  ()
  (if 
    (or
      (and 
        (= (get-candle-high 1) (get-candle-open 1))
        (= (get-candle-low 1) (get-candle-close 1)))
      (and
        (= (get-candle-high 1) (get-candle-close 1))
        (= (get-candle-low 1) (get-candle-open 1))))
    (put-candle-pattern 1 "M")
    ()))
