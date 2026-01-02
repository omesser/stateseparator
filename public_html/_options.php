            <div class="options-card">
              <h2>Parameters</h2>

              <label>Target distance:</label>
              <input type="text" name="targetDistance" placeholder="0.5E-13" />

              <label>Min probability:</label>
              <input type="text" name="minProbForState" placeholder="0" />

              <label>Target states:</label>
              <input type="text" name="targetNumberOfStates" placeholder="N²" />

              <label>Precision:</label>
              <select name="precision">
                <option value="3" selected>3</option>
                <option value="6">6</option>
                <option value="9">9</option>
                <option value="12">12</option>
                <option value="15">15</option>
              </select>

              <label>Accuracy boost:</label>
              <div class="radio-group">
                <label><input type="radio" name="accuracyBoost" value="1" /> On</label>
                <label><input type="radio" name="accuracyBoost" value="0" checked /> Off</label>
              </div>

              <div class="action-buttons">
                <button type="submit" name="separate" value="1">Separate</button>
<?php if (!empty($isExample)) : ?>
                <span class="tooltip-wrapper" title="Go to Home to input your own matrix">
                  <button type="button" class="secondary-btn" disabled>Randomize</button>
                </span>
<?php else : ?>
                <button type="submit" name="randomize" value="1" class="secondary-btn">Randomize</button>
<?php endif; ?>
              </div>
            </div>
